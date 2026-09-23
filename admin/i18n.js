// `english` is injected from translations.json by the firmware/preview builder.
const languageKey = 'switchbot-language';
let language = navigator.language.toLowerCase().startsWith('de') ? 'de' : 'en';
try {
  const stored = localStorage.getItem(languageKey);
  if (stored === 'de' || stored === 'en') language = stored;
} catch (_) { /* Browser storage is optional. */ }

function t(value) {
  if (language !== 'en' || typeof value !== 'string') return value;
  if (Object.hasOwn(english, value)) return english[value];
  // Backend validation errors can append a field name; connection errors append a cause.
  for (const prefix of ['Textfeld fehlt: ', 'Ganzzahl fehlt: ', 'Verbindung fehlgeschlagen: ']) {
    if (value.startsWith(prefix)) return english[prefix] + t(value.slice(prefix.length));
  }
  return value;
}

// Keep canonical text on each label node so switching never edits form values,
// recreates device rows, or translates user-supplied device names/passwords.
const originalText = new WeakMap();
const originalPlaceholder = new WeakMap();
function translateDOM(root) {
  const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT);
  while (walker.nextNode()) {
    const node = walker.currentNode;
    if (node.parentElement.closest('script,style,#message,#bridge,#status,#confirmationText')) continue;
    const original = originalText.get(node) ?? node.nodeValue;
    if (!Object.hasOwn(english, original.trim())) continue;
    originalText.set(node, original);
    node.nodeValue = original.replace(original.trim(), t(original.trim()));
  }
  for (const input of root.querySelectorAll('[placeholder]')) {
    const original = originalPlaceholder.get(input) ?? input.getAttribute('placeholder');
    originalPlaceholder.set(input, original);
    input.setAttribute('placeholder', t(original));
  }
}

function applyLanguage(next) {
  language = next === 'de' ? 'de' : 'en';
  document.documentElement.lang = language;
  document.getElementById('language').value = language;
  translateDOM(document);
  renderStatus();
  renderMessage();
  document.getElementById('confirmationText').textContent = t(confirmationText);
}

document.getElementById('language').onchange = event => {
  applyLanguage(event.target.value);
  try { localStorage.setItem(languageKey, language); } catch (_) { /* Optional. */ }
};
