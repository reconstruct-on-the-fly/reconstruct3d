const electron = require('electron');
const shell = require('electron').shell
const {app} = electron;
const {BrowserWindow} = electron;

let win;

function createWindow() {
  win = new BrowserWindow({width: 800, height: 600});

  win.loadURL(`file://${__dirname}/index.html`);

  //win.webContents.openDevTools();

  win.on('closed', () => {
    win = null;
  });
}


// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (win === null) {
    createWindow();
  }
});

