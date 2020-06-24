# electron-n-api
electron use n-api extend native interface. 


```
git clone git@github.com:gzx-miller/electron-n-api.git
npm install

cd addon
npm install
node-gyp rebuild
```

Modify js file ".\node_modules\bindings\bindings.js"
add one line like this, under the "defaults"

```
['module_root', 'addon', 'build', 'Release', 'bindings'],
```

then, you can run.

```
cd ../
npm run start
```





