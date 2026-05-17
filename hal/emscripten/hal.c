/*! @file
  @brief
  Hardware abstraction layer
        for Emscripten/WebAssembly

  @note Link applications with -sASYNCIFY because this HAL uses
        emscripten_sleep() to yield while mrbc_run() is idle.

  <pre>
  Copyright (C) 2015-      Kyushu Institute of Technology.
  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
  Copyright (C) 2026-      Shimane Institute for Industrial Technology.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>

/***** Local headers ********************************************************/
#include "hal.h"

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** JavaScript bridge functions ******************************************/
EM_JS(void, mrbc_hal_js_output, (int fd, const char *buf, int nbytes, int flush), {
  const root = typeof globalThis !== "undefined" ? globalThis :
    (typeof self !== "undefined" ? self :
    (typeof window !== "undefined" ? window :
    (typeof global !== "undefined" ? global : {})));
  const mod = typeof Module !== "undefined" ? Module : null;

  /**
   * Get the value of an own data property without invoking accessors.
   *
   * @param {object} object Object to inspect.
   * @param {string} name Property name to read.
   * @returns {*} Property value, or undefined when the property is missing or not a data property.
   */
  function getOwnDataProperty(object, name) {
    if (!object || typeof Object.getOwnPropertyDescriptor !== "function") return undefined;

    const descriptor = Object.getOwnPropertyDescriptor(object, name);
    if (!descriptor || !Object.prototype.hasOwnProperty.call(descriptor, "value")) return undefined;

    return descriptor.value;
  }

  const isErr = fd === 2;
  const owner = mod || root;
  const maxBufferedBytes = 4096;
  let state = owner.__mrubycHalOutputState;
  if (!state) {
    state = {
      out: [],
      err: [],
      outFlush: 0,
      errFlush: 0,
      decoder: typeof TextDecoder !== "undefined" ? new TextDecoder("utf-8") : null
    };
    owner.__mrubycHalOutputState = state;
  }

  const output = isErr ? state.err : state.out;
  const flushKey = isErr ? "errFlush" : "outFlush";
  const callbackName = isErr ? "mrubycError" : "mrubycOutput";
  const printName = isErr ? "printErr" : "print";
  const callback = getOwnDataProperty(mod, callbackName);
  const print = getOwnDataProperty(mod, printName);
  const stream = isErr ? (root.process && root.process.stderr)
                       : (root.process && root.process.stdout);
  const runtimeOutput = isErr ? (typeof err === "function" ? err : undefined)
                              : (typeof out === "function" ? out : undefined);
  const consoleObject = typeof console !== "undefined" ? console : null;
  const consoleOutput = consoleObject ? (isErr ? consoleObject.error : consoleObject.log) : null;
  const sinks = [
    { target: undefined, write: callback },
    { target: undefined, write: print },
    { target: stream, write: stream && stream.write },
    { target: undefined, write: runtimeOutput },
    { target: consoleObject, write: consoleOutput }
  ];

  /**
   * Decode buffered UTF-8 bytes into a JavaScript string.
   *
   * @param {number[]} bytes Buffered byte values.
   * @returns {string} Decoded text.
   */
  function decode(bytes) {
    if (state.decoder) {
      return state.decoder.decode(new Uint8Array(bytes));
    }

    let text = "";
    for (let i = 0; i < bytes.length; i++) {
      text += String.fromCharCode(bytes[i]);
    }
    return text;
  }

  /**
   * Emit text to the best available output sink.
   *
   * @param {string} text Decoded text to emit.
   * @returns {void}
   */
  function emitText(text) {
    for (let i = 0; i < sinks.length; i++) {
      if (typeof sinks[i].write !== "function") continue;
      sinks[i].write.call(sinks[i].target, text);
      return;
    }
  }

  /**
   * Decode and emit buffered bytes.
   *
   * @param {number[]} bytes Buffered byte values to emit.
   * @returns {void}
   */
  function emit(bytes) {
    if (bytes.length === 0) return;
    emitText(decode(bytes));
  }

  /**
   * Emit and clear the buffered bytes for the selected stream.
   *
   * @returns {void}
   */
  function flushOutput() {
    if (output.length !== 0) {
      emit(output);
      output.length = 0;
    }
  }

  /**
   * Flush buffered output after the current JavaScript turn.
   *
   * @returns {void}
   */
  function flushLater() {
    state[flushKey] = 0;
    flushOutput();
  }

  /**
   * Schedule a callback after the current JavaScript turn.
   *
   * @param {Function} callback Callback to run later.
   * @returns {void}
   */
  function scheduleLater(callback) {
    const queue = typeof queueMicrotask !== "undefined" ? queueMicrotask : root["queueMicrotask"];
    if (typeof queue === "function") {
      queue(callback);
      return;
    }

    const promise = typeof Promise !== "undefined" ? Promise : root["Promise"];
    if (promise && typeof promise.resolve === "function") {
      promise.resolve().then(callback);
      return;
    }

    const schedule = typeof setTimeout !== "undefined" ? setTimeout : root["setTimeout"];
    if (typeof schedule === "function") schedule(callback, 0);
  }

  /**
   * Schedule a deferred flush for unterminated output.
   *
   * @returns {void}
   */
  function scheduleFlush() {
    if (output.length === 0 || state[flushKey]) return;

    state[flushKey] = 1;
    scheduleLater(flushLater);
  }

  /**
   * Append bytes to the selected stream buffer and flush complete lines or full chunks.
   *
   * @param {Uint8Array} bytes Bytes read from the WebAssembly heap.
   * @returns {void}
   */
  function appendBytes(bytes) {
    for (let i = 0; i < bytes.length; i++) {
      output.push(bytes[i]);
      if (bytes[i] === 10 || output.length >= maxBufferedBytes) {
        flushOutput();
      }
    }
  }

  if (nbytes > 0) {
    appendBytes(HEAPU8.subarray(buf, buf + nbytes));
    scheduleFlush();
  }

  if (flush) flushOutput();
});

/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
/***** Global functions *****************************************************/

//================================================================
/*!@brief
  delay milliseconds

  @param ms  delay milliseconds.
*/
void mrbc_hal_delay_ms(int ms)
{
  if( ms < 0 ) ms = 0;
  emscripten_sleep((unsigned int)ms);
}


//================================================================
/*!@brief
  Write

  @param  fd      1 = stdout, 2 = stderr.
  @param  buf     pointer of buffer.
  @param  nbytes  output byte length.
*/
int mrbc_hal_write(int fd, const void *buf, int nbytes)
{
  if( nbytes < 0 ) return -1;
  if( nbytes == 0 ) return 0;
  if( buf == NULL ) return -1;

  mrbc_hal_js_output(fd, (const char *)buf, nbytes, 0);

  return nbytes;
}


//================================================================
/*!@brief
  Flush write buffer

  @param  fd      1 = stdout, 2 = stderr.
*/
int mrbc_hal_flush(int fd)
{
  mrbc_hal_js_output(fd, NULL, 0, 1);
  return 0;
}


//================================================================
/*!@brief
  abort program

  @param s  additional message.
*/
void mrbc_hal_abort(const char *s)
{
  if( s ) {
    mrbc_hal_write(2, s, strlen(s));
  }
  mrbc_hal_flush(2);

  emscripten_force_exit(EXIT_FAILURE);
  abort();
}


//================================================================
/*!@brief
  out of memory

*/
void mrbc_out_of_memory_emscripten(void)
{
  static const char msg[] = "Fatal error: Out of memory.\n";
  mrbc_hal_write(2, msg, sizeof(msg) - 1);
  mrbc_hal_abort(0);
}
