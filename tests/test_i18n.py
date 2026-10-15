import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from sync_admin import localized_script, rendered_page


class TranslationTests(unittest.TestCase):
    def test_backend_messages_have_english_translations(self):
        catalog = json.loads((ROOT / 'admin/translations.json').read_text(encoding='utf-8'))
        core = (ROOT / 'admin/config_core.h').read_text(encoding='utf-8')
        codec = (ROOT / 'admin/config_json.h').read_text(encoding='utf-8')
        runtime = (ROOT / 'admin/runtime.inc').read_text(encoding='utf-8')
        messages = re.findall(r'return "([^"]+)"', core)
        messages += re.findall(r'error=(?:std::string\()?"([^"]+)"', codec)
        messages += re.findall(r'adminError\(\d+,"([^"]+)"', runtime)
        messages += re.findall(r'(?:adminNotice|result\["message"\])="([^"]+)"', runtime)
        self.assertTrue(messages)
        self.assertEqual([], [message for message in messages if not catalog.get(message)])

    def test_firmware_page_is_self_contained(self):
        page = rendered_page()
        self.assertNotRegex(page, r'<script\s+src=')
        self.assertIn('const english=', page)
        self.assertIn('id="language"', page)

    def test_locale_selection_and_message_translation(self):
        node = shutil.which('node')
        self.assertIsNotNone(node, 'Node.js required for translation tests')
        program = r'''
const vm = require('node:vm');
const assert = require('node:assert/strict');
const source = SOURCE;
function context(browserLanguage, saved, blocked = false) {
  const c = vm.createContext({
    navigator: {language: browserLanguage, languages: browserLanguage ? [browserLanguage] : []},
    localStorage: {getItem() {if (blocked) throw Error('blocked'); return saved;}},
    document: {getElementById() {return {};}}
  });
  vm.runInContext(source, c);
  return c;
}
function contextWithLanguages(browserLanguage, browserLanguages, saved) {
  const c = vm.createContext({
    navigator: {language: browserLanguage, languages: browserLanguages},
    localStorage: {getItem() {return saved;}},
    document: {getElementById() {return {};}}
  });
  vm.runInContext(source, c);
  return c;
}
const en = context('en-GB', null);
assert.equal(vm.runInContext('language', en), 'en');
assert.equal(vm.runInContext("t('Übersicht')", en), 'Overview');
assert.equal(vm.runInContext("t('Textfeld fehlt: ssid')", en), 'Missing text field: ssid');
assert.equal(vm.runInContext("t('Verbindung fehlgeschlagen: Passwort fehlt.')", en), 'Connection failed: Password missing.');
assert.equal(vm.runInContext("t('Verbindungsprüfung fehlgeschlagen. Rollback läuft.')", en), 'Connection check failed. Restoring the previous configuration.');
assert.equal(vm.runInContext("t('custom-device')", en), 'custom-device');
assert.equal(vm.runInContext('t(42)', en), 42);
assert.equal(vm.runInContext('language', context('de-AT', null)), 'de');
assert.equal(vm.runInContext('language', context('fr-FR', null)), 'en');
assert.equal(vm.runInContext('language', contextWithLanguages('fr-FR', ['fr-FR', 'de-DE'], null)), 'de');
assert.equal(vm.runInContext('language', context('de-AT', 'en')), 'en');
assert.equal(vm.runInContext('language', context('en-US', 'de')), 'de');
assert.equal(vm.runInContext('language', context('en-US', 'de-DE')), 'de');
assert.equal(vm.runInContext('language', context('de-AT', 'invalid')), 'de');
assert.equal(vm.runInContext('language', context('en-US', null, true)), 'en');
assert.equal(vm.runInContext("t('Übersicht')", context('en-US', 'de')), 'Übersicht');
'''.replace('SOURCE', json.dumps(localized_script()))
        subprocess.run([node, '-'], input=program, text=True, check=True, encoding='utf-8')


if __name__ == '__main__':
    unittest.main()
