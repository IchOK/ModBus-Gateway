function saveConfigSlave(jSlaves, slave) {
  // Create epmty Slave config
  let jSlave = {};
  // Slave Config
  let dElements;
  dElements = slave.querySelector("header div").getElementsByTagName("label");
  for (let i = 0; i < dElements.length; i++) {
    let dInput = dElements[i].querySelector("input");
    jSlave[dInput.name] = dInput.value;
  }
  // Tag Config
  let jTags = [];
  dElements = slave.querySelectorAll("article > div");
  for (let i = 0; i < dElements.length; i++) {
    let jTag = {};
    let dInputs = dElements[i].getElementsByTagName("label");
    for (let j = 0; j < dInputs.length; j++) {
//      let dInput = dInputs[j].querySelector("input");
      for (let k = 0; k < dInputs[j].childNodes.length; k++) {
        let dInput = dInputs[j].childNodes[k];
        if (dInput.nodeName == "INPUT" || dInput.nodeName == "SELECT") {
          jTag[dInput.name] = dInput.value;
        }
      }
    }
    jTags.push(jTag);
  }
  jSlave["tags"] = jTags;
  
  jSlaves.push(jSlave);
}