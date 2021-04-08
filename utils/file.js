const outputFolder = './output';
const fs = require('fs');

const getFileHasFormatMP4 = (sid) => {
  let path = `${outputFolder}/${sid}`
  let mp4File = null
  fs.readdirSync(path).forEach(file => {
    if (file.indexOf('.mp4') > 0) {
      mp4File = file
    }
  });
  return mp4File
}

module.exports = {
  getFileHasFormatMP4
}