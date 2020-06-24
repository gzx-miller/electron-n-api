# electron-n-api
electron use n-api extend native interface. 


```
git clone git@github.com:gzx-miller/electron-n-api.git
npm install

cd addon
npm install
node-gyp rebuild
```

# Modify js file 
  ".\node_modules\bindings\bindings.js"
  add one line like this, under the "defaults"

```
['module_root', 'addon', 'build', 'Release', 'bindings'],
```


# Run

```
cd ../
npm run start
```

# Relative article

[Electron通过nodejs的Addon(n-api)实现高可扩展性接口](https://blog.csdn.net/allen8612433/article/details/106937163)


