import assert from "node:assert/strict";
import fs from "node:fs/promises";
import vm from "node:vm";

const source = await fs.readFile(new URL("../data/settings.js", import.meta.url), "utf8");

async function page(settings, status = 200) {
  const elements = new Map();
  const requests = [];
  const element = (id) => {
    if (!elements.has(id)) elements.set(id, {
      value: "", disabled: true, textContent: "", children: [], handlers: {},
      addEventListener(type, handler) { this.handlers[type] = handler; },
      replaceChildren() { this.children = []; },
      appendChild(child) { this.children.push(child); },
    });
    return elements.get(id);
  };
  vm.runInNewContext(source, {
    document: { getElementById: element, createElement: () => ({}) },
    URLSearchParams,
    setTimeout,
    window: { location: {} },
    fetch: async (url, options) => {
      requests.push({ url, options });
      return { ok: status === 200, json: async () => settings };
    },
  });
  await new Promise((resolve) => setImmediate(resolve));
  return { element, requests };
}

const options = ["none", "fadeout", "matrix", "left-right", "noise", "random"].map((id) => ({ id, label: id }));
const current = await page({ animation: "matrix", animation_options: options });
assert.equal(current.element("animation").value, "matrix");
assert.equal(current.element("animation").disabled, false);
assert.equal(current.element("animation").children.length, 6);
current.element("animation").value = "noise";
await current.element("animation-form").handlers.submit({ preventDefault() {} });
const saved = current.requests.at(-1);
assert.equal(saved.url, "/api/settings");
assert.equal(saved.options.method, "POST");
assert.equal(saved.options.body.toString(), "animation=noise");
assert.match(current.element("animation-status").textContent, /Saved/);
assert.equal(current.element("animation-save").disabled, false);
assert.equal(current.element("animation-test").disabled, false);
const requestCount = current.requests.length;
await current.element("animation").handlers.change();
assert.equal(current.requests.length, requestCount); // Test defaults to off.
current.element("animation-test").checked = true;
await current.element("animation").handlers.change();
assert.equal(current.requests.at(-1).url, "/api/animation/preview");
assert.equal(current.requests.at(-1).options.body.toString(), "animation=noise");
assert.match(current.element("animation-status").textContent, /Test started/);
assert.equal(current.element("animation").disabled, false);

current.element("animation").value = "random";
await current.element("animation").handlers.change();
assert.equal(current.requests.at(-1).options.body.toString(), "animation=random");
await current.element("animation-form").handlers.submit({ preventDefault() {} });
assert.equal(current.requests.at(-1).options.body.toString(), "animation=random");

const defaults = await page({ animation_options: options });
assert.equal(defaults.element("animation").value, "none");
const legacy = await page({});
assert.equal(legacy.element("animation-save").disabled, true);
assert.equal(legacy.element("animation-test").disabled, true);
assert.match(legacy.element("animation-status").textContent, /updated firmware/);
const failed = await page({}, 500);
assert.equal(failed.element("animation-save").disabled, true);
assert.match(failed.element("animation-status").textContent, /Failed/);
await failed.element("animation-form").handlers.submit({ preventDefault() {} });
assert.match(failed.element("animation-status").textContent, /Failed to save/);

const previewFailed = await page({ animation_options: options }, 500);
previewFailed.element("animation-test").checked = true;
previewFailed.element("animation").disabled = false;
await previewFailed.element("animation").handlers.change();
assert.match(previewFailed.element("animation-status").textContent, /Failed to start test/);
assert.equal(previewFailed.element("animation").disabled, false);

console.log("Settings UI tests passed");
