let napi = require('bindings')('napi');

let app = {
    run: function() {
        // js player in iframe,  127.0.0.1 test-y.xxx.com
        //let ifm1 = document.createElement('iframe');
        //ifm1.setAttribute("width", 600);
        //ifm1.setAttribute("height", 400);
        //ifm1.setAttribute("src", "http://test-y.xxx.com/jsplayer.html"); 
        //let ifm2 = document.createElement('iframe');
        //ifm2.setAttribute("width", 600);
        //ifm2.setAttribute("height", 400);
        //ifm2.setAttribute("src", "http://test-y.xxx.com/jsplayer.html"); 
        //let body = document.getElementsByTagName("body")[0];
        //body.appendChild(ifm1);
        //body.appendChild(ifm2);
        
        // 1. call native function
        console.log("RetStr: " + napi.RetStr());
        console.log("Add: " + napi.Add(1, 2)); 

        // 2. callback from native
        napi.RunCallback((msg) => {
            console.log("RunCallback: " + msg);
        });

        // 3. create object from native
        let obj1 = napi.CreateObject("MyObj");
        console.log("CreateObject: " + obj1.msg);
        
        // 4. create function from native
        let func = napi.CreateFunction();
        console.log("CreateFunction: " + func());
        
        // 5. create object with function return promise, 
        //    because promise need something handle callback.
        let doer1 = napi.CreateDoer();
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
        let doer2 = napi.CreateDoer();
        doer2.todo().then((data)=>{
            console.log("Promise2.then data param: " + data);
            return data + 1;
        });
        
        // 6. Extensible api with json(cmd)
        global.onSvcRsp = function onSvcRsp(resp) {
            console.log("RegSvcRsp: resp:" + resp);
            setTimeout(() => {
                napi.UnregSvcRsp();
                console.log("UnregSvcRsp");
            }, 100);
        }
        napi.RegSvcRsp(global.onSvcRsp);
        let reqStr = JSON.stringify({
            "type":"req",
            "cmd":"get_user_info",
            "params":{ "user_id": 100001 }
        });
        console.log("SendSvcReq: " + reqStr);
        let ret2 = napi.SendSvcReq(reqStr);
        console.log("SendSvcReq ret: " + ret2);

        // 7. For play video
        // status (1: frame, 0: complete, -1: failed)
        var cnv = document.getElementsByTagName("canvas")[0];
        var bcr = cnv.getBoundingClientRect();
        var ctx=cnv.getContext("2d");
        cnv.width = bcr.width;
        cnv.height = bcr.height;
        cnv.style = `
          image-rendering: optimizeSpeed;
          image-rendering: -moz-crisp-edges;
          image-rendering: -webkit-optimize-contrast;
          image-rendering: -o-crisp-edges;
          image-rendering: optimize-contrast;
          image-rendering: crisp-edges;
          image-rendering: pixelated;
          -ms-interpolation-mode: nearest-neighbor;
        `;
        ctx.imageSmoothingEnabled = false;

        global.onReceiveFrame = function onReceiveFrame(status) {
            if (status === 1) {
                ctx.putImageData(global.frameBuf, 0, 0); 
            } else {
                setTimeout(() => {
                    napi.StopDecode(global.codeIndex);
                    napi.UnRegReceiveFrame();
                }, 100);
            }
        }
        napi.RegRcvFrame(global.onReceiveFrame);
        global.frameBuf = ctx.createImageData(bcr.width, bcr.height);
        global.codeIndex = napi.StartDecode("a.h265", global.frameBuf.buffer);
    }
}

window.onload = function () {
    app.run();
}

module.exports = exports = app;