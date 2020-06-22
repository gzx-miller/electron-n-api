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

var fetcher1 = napi.CreateFetcher();
let promise1 = fetcher1.fetch();
promise1.then(data => {
    console.log("Promise1.0.then data param: " + data);
});
promise1.then((data)=>{
    console.log("Promise1.then data param: " + data);
    return data + 1;
}).then((data)=>{
    console.log("Promise1.2.then data param: " + data);
    return data + 1;
}).then((data)=>{
    console.log("Promise1.3.then data param: " + data);
    return data + 1;
});

var fetcher2 = napi.CreateFetcher();
fetcher2.fetch().then((data)=>{
    console.log("Promise2.then data param: " + data);
    return data + 1;
});

var fetcher3 = napi.CreateFetcher();
fetcher3.fetch().then((data)=>{
    console.log("Promise3.then data param: " + data);
    return data + 1;
});
