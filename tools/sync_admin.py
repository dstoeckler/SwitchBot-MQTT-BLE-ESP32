"""Embed the offline admin assets in both standalone firmware variants."""
from pathlib import Path
import gzip
import re
import json

ROOT=Path(__file__).resolve().parents[1]
SKETCH=ROOT/'Arduino IDE Files/SwitchBot-BLE2MQTT-ESP32.ino'

def localized_script():
    translations=json.loads((ROOT/'admin/translations.json').read_text(encoding='utf-8'))
    return 'const english='+json.dumps(translations,ensure_ascii=False)+';\n'+(ROOT/'admin/i18n.js').read_text(encoding='utf-8')

def rendered_page():
    return (ROOT/'admin/index.html').read_text(encoding='utf-8').replace('<script src="i18n.js"></script>', '<script>\n'+localized_script()+'\n</script>')

def generated():
    core=(ROOT/'admin/config_core.h').read_text(encoding='utf-8').replace('#pragma once\n','')
    codec=(ROOT/'admin/config_json.h').read_text(encoding='utf-8').replace('#pragma once\n','').replace('#include "config_core.h"\n','')
    ui=bytearray(gzip.compress(rendered_page().encode('utf-8'),mtime=0))
    # Python 3.11/3.12 delegate mtime=0 to zlib, which emits a host OS byte.
    # Normalize it to the portable "unknown" value used by Python 3.13+.
    ui[9]=255
    array='static const uint8_t adminPage[] PROGMEM = {\n'+',\n'.join(','.join(str(b) for b in ui[i:i+24]) for i in range(0,len(ui),24))+'\n};\n'
    return {'CORE':core+codec,'UI':array,'RUNTIME':(ROOT/'admin/runtime.inc').read_text(encoding='utf-8')}

def rendered():
    source=SKETCH.read_text(encoding='utf-8')
    for name,body in generated().items():
        pattern=r'// BEGIN GENERATED ADMIN '+name+r'\n.*?// END GENERATED ADMIN '+name
        source,count=re.subn(pattern,lambda m:'// BEGIN GENERATED ADMIN '+name+'\n'+body+'// END GENERATED ADMIN '+name,source,flags=re.S)
        if count!=1:raise RuntimeError('Missing generated marker: '+name)
    return source

if __name__=='__main__':
    import sys
    result=rendered()
    other=ROOT/'PlatformIO Files/SwitchBot-BLE2MQTT-ESP32/src/SwitchBot-BLE2MQTT-ESP32.cpp'
    if '--check' in sys.argv:
        assert SKETCH.read_text(encoding='utf-8')==result,'Admin embed is stale; run tools/sync_admin.py'
        assert other.read_bytes()==SKETCH.read_bytes(),'Firmware variants differ'
    else:
        for path in (SKETCH,other):path.write_text(result,encoding='utf-8',newline='\n')
