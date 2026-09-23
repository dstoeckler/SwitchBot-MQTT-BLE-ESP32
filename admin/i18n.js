// `english` is injected from translations.json by the firmware/preview builder.
const languageKey = 'switchbot-language';
const supportedLanguages = ['de', 'en'];

function resolveLanguage(candidate) {
  if (typeof candidate !== 'string') return null;
  const tag = candidate.toLowerCase();
  if (supportedLanguages.includes(tag)) return tag;
  const base = tag.split('-')[0];
  return supportedLanguages.includes(base) ? base : null;
}

function preferredBrowserLanguage() {
  const candidates = Array.isArray(navigator.languages) && navigator.languages.length ? navigator.languages : [navigator.language];
  for (const candidate of candidates) {
    const resolved = resolveLanguage(candidate);
    if (resolved) return resolved;
  }
  return 'en';
}

let language = preferredBrowserLanguage();
try {
  const stored = resolveLanguage(localStorage.getItem(languageKey));
  if (stored) language = stored;
} catch (_) { /* Browser storage is optional. */ }

function normalizeUmlauts(value) {
  return value
    .replace(/ä/g, 'ae')
    .replace(/ö/g, 'oe')
    .replace(/ü/g, 'ue')
    .replace(/Ä/g, 'Ae')
    .replace(/Ö/g, 'Oe')
    .replace(/Ü/g, 'Ue')
    .replace(/ß/g, 'ss');
}

function englishFor(value) {
  if (Object.hasOwn(english, value)) return english[value];
  const normalized = typeof value.normalize === 'function' ? value.normalize('NFC') : value;
  if (normalized !== value && Object.hasOwn(english, normalized)) return english[normalized];
  const fallback = normalizeUmlauts(normalized);
  if (fallback !== normalized && Object.hasOwn(english, fallback)) return english[fallback];
  return undefined;
}

const variableMessagePrefixes = Object.keys(english)
  .filter(key => key.endsWith(': '))
  .sort((a, b) => b.length - a.length);

function t(value) {
  if (language !== 'en' || typeof value !== 'string') return value;
  const direct = englishFor(value);
  if (direct !== undefined) return direct;
  // Backend validation errors can append a field name; connection errors append a cause.
  for (const prefix of variableMessagePrefixes) {
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
