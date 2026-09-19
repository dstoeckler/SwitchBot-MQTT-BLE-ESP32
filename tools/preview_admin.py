"""Local UI fixture. No ESP32, network settings or MQTT broker are touched."""
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path
import json

ROOT=Path(__file__).resolve().parents[1]
CONFIG=dict(version=1,host='living-room',ssid='Zuhause',mqttHost='192.168.1.20',mqttUser='switchbot',topic='switchbot',port=1883,scanSeconds=120,rescanSeconds=10800,retries=5,staticAddress=False,ip='192.168.1.50',gateway='192.168.1.1',subnet='255.255.255.0',dns='192.168.1.1',devices=[dict(id='stehlampe',mac='AA:BB:CC:DD:EE:01',type='bot',entity='switch'),dict(id='wohnzimmer',mac='AA:BB:CC:DD:EE:02',type='curtain',entity='switch')])

class Handler(BaseHTTPRequestHandler):
    def reply(self,status,body,content='application/json'):
        data=body.encode() if isinstance(body,str) else json.dumps(body).encode()
        self.send_response(status);self.send_header('Content-Type',content);self.send_header('Content-Length',str(len(data)));self.end_headers();self.wfile.write(data)

    def do_GET(self):
        if self.path=='/':self.reply(200,(ROOT/'admin/index.html').read_text(encoding='utf-8'),'text/html; charset=utf-8')
        elif self.path=='/api/config':self.reply(200,dict(config=CONFIG,csrf='preview-token',trial=False))
        elif self.path=='/api/status':self.reply(200,dict(wifi=True,mqtt=True,ip='192.168.1.50',uptime=86400,heap=124000,devices=len(CONFIG['devices']),ap=False,trial=False,notice='Lokale Vorschau – keine Hardware verbunden.'))
        else:self.reply(404,dict(error='Nicht gefunden'))

    def do_POST(self):
        body=self.rfile.read(int(self.headers.get('Content-Length','0')))
        if self.headers.get('X-CSRF-Token')!='preview-token':return self.reply(403,dict(error='Ungültige Sitzung'))
        if self.path=='/update':return self.reply(200,'OK','text/plain')
        try:doc=json.loads(body)
        except ValueError:return self.reply(400,dict(error='JSON ungültig'))
        if self.path=='/api/config':
            (ROOT/'build-admin-preview-request.json').write_text(json.dumps(doc,indent=2),encoding='utf-8')
            self.reply(200,dict(message='Vorschau: Konfiguration empfangen. Es wurde keine Hardware geändert.'))
        elif self.path=='/api/test':self.reply(202,{}) if any(d['id']==doc.get('id') for d in CONFIG['devices']) else self.reply(404,dict(error='Gerät zuerst speichern.'))
        elif self.path in ('/api/password','/api/restart'):self.reply(200,{})
        else:self.reply(404,dict(error='Nicht gefunden'))

if __name__=='__main__':HTTPServer(('127.0.0.1',8765),Handler).serve_forever()
