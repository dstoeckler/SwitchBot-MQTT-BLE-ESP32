"""Exercise the firmware's subscription handshake against retained MQTT replay."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

from test_regressions import SOURCE, block


class StartupCommands(unittest.TestCase):
    def test_retained_commands_and_reconnect(self):
        source = SOURCE.read_text(encoding='utf-8')
        program = r'''
#include <cassert>
#include <functional>
#include <map>
#include <string>
struct String : std::string {
  using std::string::string;
  bool isEmpty() const { return empty(); }
};
using MessageReceivedCallback = std::function<void(const String&)>;
static std::string commandSubscriptionMarker;
static std::map<std::string, bool> commandTopicReady;
struct Client {
  std::map<std::string, MessageReceivedCallback> callbacks;
  std::map<std::string, String> retained;
  std::map<std::string, String> pending;
  bool subscribeOK = true, publishOK = true;
  bool subscribe(const char* topic, MessageReceivedCallback callback) {
    if (!subscribeOK) return false;
    // Match EspMQTTClient: it retains existing callbacks across reconnects.
    if (!callbacks.count(topic)) callbacks[topic] = callback;
    if (retained.count(topic)) callbacks[topic](retained[topic]);
    return true;
  }
  bool publish(const char* topic, const char* payload, bool retain) {
    assert(!retain); // The handshake must never replace retained device state.
    if (!publishOK) return false;
    pending[topic] = String(payload);
    return true;
  }
  void deliver(const char* topic, const char* payload) { callbacks.at(topic)(String(payload)); }
  void acknowledge(const char* topic) { callbacks.at(topic)(pending.at(topic)); }
} client;
'''
        program += block(source, 'bool subscribeCommandTopic(')
        program += r'''
int main() {
  int actions = 0;
  auto action = [&](const String&) { ++actions; };
  for (const char* payload : {"PRESS", "ON", "OFF", "5", "{\"id\":\"bot\",\"hold\":5}"}) {
    commandSubscriptionMarker = "__bridge_ready_boot";
    commandTopicReady.clear(); client.callbacks.clear();
    client.retained["bot/set"] = String(payload);
    assert(subscribeCommandTopic("bot/set", action));
    assert(actions == 0); // Old action on boot is never queued.
    client.deliver("bot/set", "__bridge_ready_previous_boot");
    assert(actions == 0);
    client.acknowledge("bot/set"); assert(actions == 0);
    client.deliver("bot/set", ""); assert(actions == 0);
    client.deliver("bot/set", "PRESS"); assert(actions == 1);
    commandSubscriptionMarker = "__bridge_ready_reconnect";
    commandTopicReady.clear();
    assert(subscribeCommandTopic("bot/set", action));
    assert(actions == 1); // Retained replay on reconnect is also ignored.
    client.deliver("bot/set", "__bridge_ready_boot"); assert(actions == 1);
    client.acknowledge("bot/set");
    client.deliver("bot/set", "ON"); assert(actions == 2);
    actions = 0;
  }
  client.publishOK = false;
  assert(!subscribeCommandTopic("bot/set", action));
  client.deliver("bot/set", "PRESS"); assert(actions == 0);
  client.subscribeOK = false;
  assert(!subscribeCommandTopic("bot/set", action));
  client.deliver("bot/set", "ON"); assert(actions == 0);
  client.subscribeOK = client.publishOK = true;
  int state = 0;
  client.retained["bot/assumedstate"] = "ON";
  client.subscribe("bot/assumedstate", [&](const String&) { ++state; });
  assert(state == 1); // Regular state restoration remains functional.
}
'''
        compiler = shutil.which('g++')
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as temp:
            cpp = Path(temp) / 'startup.cpp'
            exe = Path(temp) / 'startup.exe'
            cpp.write_text(program, encoding='utf-8')
            subprocess.run([compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror', str(cpp), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)

    def test_all_command_subscriptions_are_guarded(self):
        source = SOURCE.read_text(encoding='utf-8')
        for kind in ['bot', 'curtain', 'plug', 'meter', 'contact', 'motion']:
            self.assertIn(f'subscribeCommandTopic(({kind}Topic + aDevice + "/set").c_str(),', source)
        for topic in ['setModeStdStr', 'setHoldStdStr', 'holdPressStdStr']:
            self.assertIn(f'subscribeCommandTopic({topic}.c_str(),', source)
        connection = block(source, 'void onConnectionEstablished() {')
        self.assertIn('adminRandom()', connection)
        self.assertIn('commandTopicReady.clear()', connection)
