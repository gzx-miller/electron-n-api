var napi = require('bindings')('napi');

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
