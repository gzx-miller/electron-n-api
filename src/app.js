var napi = require('bindings')('napi');

var app = {
    run: function() {
        console.log("RetStr: " + napi.RetStr());
        console.log("Add: " + napi.Add(1, 2)); 
        napi.RunCallback(function(msg){
            console.log("RunCallback: " + msg);
        });
        
        var obj1 = napi.CreateObject("MyObj");
        console.log("CreateObject: " + obj1.msg);
        
        var func = napi.CreateFunction();
        console.log("CreateFunction: " + func());
    }
}

window.onload = function () {
    app.run();
}

module.exports = exports = app;