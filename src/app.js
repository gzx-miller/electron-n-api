var napi = require('bindings')('napi');

var app = {
    run: function() {
        // js player in iframe
        // let ifm = document.createElement('iframe');
        // ifm.setAttribute("width", 600);
        // ifm.setAttribute("height", 400);
        // ifm.setAttribute("src", "http://test-y.xxx.com/player3.html");
        // let body = document.getElementsByTagName("body")[0];
        // body.appendChild(ifm);

        console.log("RetStr: " + napi.RetStr());
        console.log("Add: " + napi.Add(1, 2)); 
        napi.RunCallback((msg) => {
            console.log("RunCallback: " + msg);
        });
        
        var obj1 = napi.CreateObject("MyObj");
        console.log("CreateObject: " + obj1.msg);
        
        var func = napi.CreateFunction();
        console.log("CreateFunction: " + func());
        
        var doer1 = napi.CreateDoer();
        let promise1 = doer1.todo();
        promise1.then(data => {
            console.log("Promise1.0.then data param: " + data);
        });
        promise1.then((data)=>{
            console.log("Promise1.then data param: " + data);
            return data + 1;
        }).then((data)=>{
            console.log("Promise1.2.then data param: " + data);
            return data + 1;
        });
        
        var doer2 = napi.CreateDoer();
        doer2.todo().then((data)=>{
            console.log("Promise2.then data param: " + data);
            return data + 1;
        });
        
        var doer3 = napi.CreateDoer();
        doer3.todo().then((data)=>{
            console.log("Promise3.then data param: " + data);
            return data + 1;
        });


        function onSvcRsp(resp) {
            console.log("RegSvcRsp: resp:" + resp);
        }
        global.onSvcRsp = onSvcRsp;
        var ret = napi.RegSvcRsp(global.onSvcRsp);
        
        var reqStr = JSON.stringify({
            "type":"req",
            "cmd":"get_user_info",
            "params":{ "user_id": 100001 }
        });
        console.log("SendSvcReq: " + reqStr);
        ret = napi.SendSvcReq(reqStr);
        console.log("SendSvcReq ret: " + ret);
        
        setTimeout(() => {
            napi.UnregSvcRsp();
            console.log("UnregSvcRsp");
            console.log("done");
        }, 4000);
    }
}

window.onload = function () {
    app.run();
}

module.exports = exports = app;