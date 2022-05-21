function createNewConfigSlave(that) {
  console.log(that);
  let btnFunc = that.getAttribute("onclick");
  let thatParent = that.parentElement;
  that.remove();
  createConfigSlave(thatParent, {tags:[]});
  createNewButton(thatParent, btnFunc, "New Slave", "secondary");
}

function createNewConfigTag(that) {
  console.log(that);
  let btnFunc = that.getAttribute("onclick");
  let thatParent = that.parentElement;
  that.remove();
  createConfigTag(thatParent, {});
  createNewButton(thatParent, btnFunc, "New Tag", "secondary");
}

function createNewButton(root, func, btnText, btnClass) {
  let btn = document.createElement("button");
  btn.setAttribute("onclick", func);
  btn.classList.add(btnClass);
  btn.innerText = btnText;
  root.appendChild(btn);
}

function createConfigSlave(slaves, config) {
  let slave = document.createElement("article");
  // Create Header as Slave Config
  let x = document.createElement("header");
  let data = document.createElement("div");
  data.classList.add("grid");
  createConfigText(data, config, "name", "Name");
  createConfigText(data, config, "id", "MB-ID");
  x.appendChild(data);
  slave.appendChild(x);
  // Create existing Tags
  config.tags.forEach((_tagConfig) => {
    createConfigTag(slave, _tagConfig);
  });
  // Create Tag-Creation-Button
  createNewButton(slave, "createNewConfigTag(this)", "New Tag", "secondary");
  createNewButton(slave, "this.parentElement.remove()", "Delete Slave", "attention");
  slaves.appendChild(slave);
}

function createConfigLabel(tag, tagName, tagText) {
  let y = document.createElement("label");
  y.setAttribute("for", tagName);
  y.innerText = tagText;
  tag.appendChild(y);
  return y;
}

function createConfigText(tag, config, typeStr, typeText) {
  let y = createConfigLabel(tag, typeStr, typeText);
  let x = document.createElement("input");
  x.id = typeStr;
  x.setAttribute("name", typeStr);
  x.type = "text";
  if (typeStr in config) {
    x.value = config[typeStr];
  }
  y.appendChild(x);
}

function createConfigTypeOld(tag, config) {
  let typeStr = "type";
  let y = createConfigLabel(tag, typeStr, "Datatype");
  let x = document.createElement("input");
  x.id = typeStr;
  x.setAttribute("name", typeStr);
  x.type = "text";
  if (typeStr in config) {
    x.value = config[typeStr];
  }
  y.appendChild(x);
}

function addSelectOption(list, value) {
  let opt = document.createElement("option");
  opt.value = value;
  opt.text = value;
  list.appendChild(opt);
}

function createConfigType(tag, config) {
  let typeStr = "type";
  let y = createConfigLabel(tag, typeStr, "Datatype");
  let x = document.createElement("select");
  x.id = typeStr;
  x.setAttribute("name", typeStr);
  addSelectOption(x, "bool");
  addSelectOption(x, "word");
  addSelectOption(x, "int16");
  addSelectOption(x, "uint16");
  addSelectOption(x, "dword");
  addSelectOption(x, "int32");
  addSelectOption(x, "uint32");
  addSelectOption(x, "float");
  addSelectOption(x, "qword");
  addSelectOption(x, "int64");
  addSelectOption(x, "uint64");
  addSelectOption(x, "double");
  if (typeStr in config) {
    x.value = config[typeStr];
  }
  y.appendChild(x);
}



function createConfigBE(tag, config) {
  let typeStr = "be";
  let y = createConfigLabel(tag, typeStr, "Big-Ending");
  let x = document.createElement("select");
  x.id = typeStr;
  x.setAttribute("name", typeStr);
  addSelectOption(x, "true");
  addSelectOption(x, "false");
  if (typeStr in config) {
    x.value = config[typeStr];
  }
  y.appendChild(x);
}

function createConfigFunc(tag, config) {
  let typeStr = "func";
  let y = createConfigLabel(tag, typeStr, "MB-Register");
  let x = document.createElement("select");
  x.id = typeStr;
  x.setAttribute("name", typeStr);
  addSelectOption(x, "coil");
  addSelectOption(x, "ists");
  addSelectOption(x, "hreg");
  addSelectOption(x, "ireg");
  if (typeStr in config) {
    x.value = config[typeStr];
  }
  y.appendChild(x);
}

function createConfigTag(slave, config) {
  let tag = document.createElement("div");
  tag.setAttribute("name", config.name);
  tag.classList.add("grid");
  
  createConfigText(tag, config, "name", "Name");
  createConfigFunc(tag, config);
  createConfigText(tag, config, "adr", "MB-Address");
  createConfigBE(tag, config);
  createConfigType(tag, config);
  createConfigText(tag, config, "unit", "Unit");
  createNewButton(tag, "this.parentElement.remove()", "Delete Tag", "attention");
  
  slave.appendChild(tag);
}