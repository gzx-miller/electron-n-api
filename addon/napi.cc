#include <windows.h>
#include <string>
#include <assert.h>
#include <stdlib.h>
#include <node_api.h>
#include <stdio.h>
#include <ctime>


#define NAPI_DESC(name, func) \
  napi_property_descriptor{ name, 0, func, 0, 0, 0, napi_default, 0 }

#define NAPI_DESC_Data(name, func, data) \
  napi_property_descriptor{ name, 0, func, 0, 0, 0, napi_default, data }

#define CHECK(expr) \
  { \
    if ((expr == napi_ok) == 0) { \
      fprintf(stderr, "[Err] %s:%d: %s\n", __FILE__, __LINE__, #expr); \
      fflush(stderr); \
      abort(); \
    } \
}

#define Logout(str) { \
    fprintf(stderr, "%s:%d: %s \n", __FILE__, __LINE__, str); \
    fflush(stderr); \
  }
#define LogoutInt(nValue) { \
  fprintf(stderr, "%s:%d: %ld \n", __FILE__, __LINE__, nValue); \
  fflush(stderr); \
}
///////////////////////////////////////////////////////////////////////
// 1. Return a string.
napi_value RetStr(napi_env env, napi_callback_info info) {
  napi_value world;
  CHECK(napi_create_string_utf8(env, "world", 5, &world));
  return world;
}

///////////////////////////////////////////////////////////////////////
// 2. Return sum of two params.
napi_value Add(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2];
  CHECK(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

  double value0, value1;
  CHECK(napi_get_value_double(env, args[0], &value0));
  CHECK(napi_get_value_double(env, args[1], &value1));

  napi_value sum;
  CHECK(napi_create_double(env, value0 + value1, &sum));
  return sum;
}

///////////////////////////////////////////////////////////////////////
// 3. Call the callback from params, and get the result.
napi_value RunCallback(napi_env env, const napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
  napi_value cb = args[0];

  napi_value argv[1];
  CHECK(napi_create_string_utf8(env, "test RunCallback", NAPI_AUTO_LENGTH, argv));
  napi_value global;
  CHECK(napi_get_global(env, &global));

  napi_value result;
  CHECK(napi_call_function(env, global, cb, 1, argv, &result));

  return nullptr;
}

///////////////////////////////////////////////////////////////////////
// 4. Return a object, with a property from js.
napi_value CreateObject(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

  napi_value obj;
  CHECK(napi_create_object(env, &obj));
  CHECK(napi_set_named_property(env, obj, "msg", args[0]));
  return obj;
}

///////////////////////////////////////////////////////////////////////
// 5. Return a function, for js call.
napi_value MyFunction(napi_env env, napi_callback_info info) {
  napi_value str;
  CHECK(napi_create_string_utf8(env, "do Something", NAPI_AUTO_LENGTH, &str));
  return str;
}
napi_value CreateFunction(napi_env env, napi_callback_info info) {
  napi_value fn;
  CHECK(napi_create_function(
      env, "theFunction", NAPI_AUTO_LENGTH, MyFunction, nullptr, &fn));
  return fn;
}

///////////////////////////////////////////////////////////////////////
// 7. Return a Doer for js call todo with promise.
struct AddonData {
  AddonData(): work(nullptr), deferred(nullptr), dataValue(-1) {};
  napi_async_work work;
  napi_deferred deferred;
  int dataValue;
};

static void ExecuteWork(napi_env env, void* data) {
  static int s_work_count = 0;
  if (data != nullptr) {
    ((AddonData*)data)->dataValue = ++ s_work_count;
  }
}

static void WorkComplete(napi_env env, napi_status status, void* data) {
  if (status != napi_ok) {
    return;
  }

  if (data != nullptr) {
    AddonData* addon_data = (AddonData*)data;
    napi_value jsValue;
    CHECK(napi_create_int32(env, addon_data->dataValue, &jsValue));
    CHECK(napi_resolve_deferred(env, addon_data->deferred, jsValue));
    CHECK(napi_delete_async_work(env, addon_data->work));
    addon_data->work = NULL;
    addon_data->deferred = NULL;
    delete addon_data;
  }
}

napi_value todo(napi_env env, napi_callback_info info) {
  napi_value work_name, promise;
  AddonData* addon_data;
  CHECK(napi_get_cb_info(env, info, NULL, NULL, NULL, (void**)(&addon_data)));
  CHECK(napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &work_name));
  CHECK(napi_create_promise(env, &(addon_data->deferred), &promise));
  CHECK(napi_create_async_work(env, NULL, work_name, ExecuteWork, WorkComplete, 
    addon_data, &(addon_data->work)));
  CHECK(napi_queue_async_work(env, addon_data->work));
  return promise;
}

napi_value CreateDoer(napi_env env, napi_callback_info info) {
  napi_value doer;
  CHECK(napi_create_object(env, &doer));
  AddonData* addon_data = new AddonData();
  napi_property_descriptor desc = NAPI_DESC_Data("todo", todo, addon_data);
  CHECK(napi_define_properties(env, doer, 1, &desc));
  return doer;
}

///////////////////////////////////////////////////////////////////////
// RegSvcRsp, SendSvcReq
struct CallbackData{
  CallbackData(): work(nullptr), tsfn(nullptr) {}
  napi_async_work work;
  napi_threadsafe_function tsfn;
};
CallbackData* g_cb_data;
napi_value g_js_cb;
static void PostSvcRsp(napi_env env, napi_value js_cb, void* context, void* pData) {
  Logout((char*)pData);
  napi_value argv[1];
  CHECK(napi_create_string_utf8(env, (char*)pData, NAPI_AUTO_LENGTH, argv));
  napi_value undefined;
  CHECK(napi_get_undefined(env, &undefined));
  napi_value result;
  CHECK(napi_call_function(env, undefined, js_cb, 1, argv, &result));
  Logout(result);
}

napi_value RegSvcRsp(napi_env env, napi_callback_info info) {
  napi_value work_name;
  CallbackData* cb_data;
  size_t argc = 1;
  CHECK(napi_get_cb_info(env, info, &argc, &g_js_cb, NULL, (void**)(&cb_data)));
  CHECK(napi_create_string_utf8(env, "CallAsyncWork", NAPI_AUTO_LENGTH, &work_name));
  CHECK(napi_create_threadsafe_function(env, g_js_cb, NULL, work_name,
        0, 1, NULL, NULL, NULL, PostSvcRsp, &(cb_data->tsfn)));
  return nullptr;
}

napi_value UnregSvcRsp(napi_env env, napi_callback_info info) {
  g_js_cb = nullptr;
  CHECK(napi_release_threadsafe_function(g_cb_data->tsfn, napi_tsfn_release));
  // 这里没启动work, 所以不需要删除work
  // CHECK(napi_delete_async_work(env, g_cb_data->work));
  g_cb_data->work = nullptr;
  g_cb_data->tsfn = nullptr;
  delete g_cb_data;
  return nullptr;
}

napi_value SendSvcReq(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  CHECK(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
  size_t realSize = 0;
  CHECK(napi_get_value_string_utf8(env, args[0], nullptr, realSize, &realSize));
  char* buf = new char[realSize];
  CHECK(napi_get_value_string_utf8(env, args[0], buf, realSize, &realSize));

  Logout(buf);
  delete buf;
  // send resp in another thread,
  char* retStr = "{\"type\":\"rsp\",\"cmd\":\"get_user_info\",\"params\":{ \"user_name\": \"miller\" }}";
  CHECK(napi_call_threadsafe_function(g_cb_data->tsfn, (void*)retStr, napi_tsfn_blocking));

  // or return data directly
  napi_value retData;
  CHECK(napi_create_string_utf8(env, retStr, NAPI_AUTO_LENGTH, &retData));
  return retData;
}

///////////////////////////////////////////////////////////////////////
// StartDecode(onJsRcvFrame), StopDecode(index)
// A. js注册回调RegReceiveframe
//    解析出onJsRcvFrame
// B. Js开始播放StartPlay
//    1. 解析出filePath
//    2. 解析出jsBuf (怎么解析, 怎么操作?)
//    3. 新建frameBuf和onNapiRcvFrame
//    4. 调用dll中的StartDecode(filePath, frameBuf, onNapiRcvFrame)
//    5. dll回调onNapiRcvFrame
//       a. 拷贝frameBuf到jsBuf
//       b. 触发onJsRcvFrame
// C. 完毕或中断时, 调用StopPlay和StopDecode
CallbackData* g_rcv_frame;
napi_value g_rev_frame_cb;
static void onJsRcvFrame(napi_env env, napi_value js_cb, void* context, void* pData) {
 napi_value argv[1];
 double dbValue = *(double*)pData;
 CHECK(napi_create_double(env, dbValue, argv));
 napi_value undefined;
 CHECK(napi_get_undefined(env, &undefined));
 napi_value result;
 CHECK(napi_call_function(env, undefined, js_cb, 1, argv, &result));
}

napi_value RegRcvFrame(napi_env env, napi_callback_info info) {
    napi_value work_name;
    CallbackData* cb_data;
    size_t argc = 1;
    CHECK(napi_get_cb_info(env, info, &argc, &g_rev_frame_cb, NULL, (void**)(&cb_data)));
    CHECK(napi_create_string_utf8(env, "tryDecode", NAPI_AUTO_LENGTH, &work_name));
    CHECK(napi_create_threadsafe_function(env, g_rev_frame_cb, NULL, work_name,
        0, 1, NULL, NULL, NULL, onJsRcvFrame, &(cb_data->tsfn)));
    return nullptr;
}

napi_value StartDecode(napi_env env, const napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2];
  CHECK(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));               // get param

  size_t realSize = 0;
  CHECK(napi_get_value_string_utf8(env, args[0], nullptr, realSize, &realSize));   // get char* from js param
  char* buf = new char[realSize];
  CHECK(napi_get_value_string_utf8(env, args[0], buf, realSize, &realSize));

  bool isArrayBuf = false;
  napi_is_arraybuffer(env, args[1], &isArrayBuf);
  if (!isArrayBuf) {
    Logout("[Err] StartDecode params 2 is not ArrayBuffer.");
    return nullptr;
  }
  
  double status = 1;
  CHECK(napi_call_threadsafe_function(g_cb_data->tsfn, (void*)&status, napi_tsfn_blocking));
  return nullptr;
}
//
//napi_value StopDecode(napi_env env, const napi_callback_info info) {
//  // 1. 停止解码
//  return nullptr;
//}
//
//napi_value UnRegRcvFrame(napi_env env, const napi_callback_info info) {
//  g_rev_frame_cb = nullptr;
//  CHECK(napi_release_threadsafe_function(g_cb_data->tsfn, napi_tsfn_release));
//  g_cb_data->work = nullptr;
//  g_cb_data->tsfn = nullptr;
//  delete g_cb_data;
//  return nullptr;
//}

///////////////////////////////////////////////////////////////////////
// Init, exports functions.
napi_value Init(napi_env env, napi_value exports) {
  Logout("napi init");
  napi_property_descriptor desc;
  
  desc = NAPI_DESC("RetStr", RetStr);
  CHECK(napi_define_properties(env, exports, 1, &desc));

  desc = NAPI_DESC("Add", Add);
  CHECK(napi_define_properties(env, exports, 1, &desc));

  desc = NAPI_DESC("RunCallback", RunCallback);
  CHECK(napi_define_properties(env, exports, 1, &desc));

  desc = NAPI_DESC("CreateObject", CreateObject);
  CHECK(napi_define_properties(env, exports, 1, &desc));

  desc = NAPI_DESC("CreateFunction", CreateFunction);
  CHECK(napi_define_properties(env, exports, 1, &desc));
  
  desc = NAPI_DESC("CreateDoer", CreateDoer);
  CHECK(napi_define_properties(env, exports, 1, &desc));

  g_cb_data = new CallbackData();  
  desc = NAPI_DESC_Data("RegSvcRsp", RegSvcRsp, g_cb_data);
  CHECK(napi_define_properties(env, exports, 1, &desc));
  
  desc = NAPI_DESC("UnregSvcRsp", UnregSvcRsp);
  CHECK(napi_define_properties(env, exports, 1, &desc));

  desc = NAPI_DESC("SendSvcReq", SendSvcReq);
  CHECK(napi_define_properties(env, exports, 1, &desc));
  
  //g_rcv_frame = new CallbackData();
  //desc = NAPI_DESC_Data("StartDecode", StartDecode, g_rcv_frame);
  //CHECK(napi_define_properties(env, exports, 1, &desc));

  //desc = NAPI_DESC("StopDecode", StopDecode);
  //CHECK(napi_define_properties(env, exports, 1, &desc));

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
