
function reconstructStart() {
    const shell = require('electron').shell;
    const ChildProcess = require('child_process');

    var reconstructCommand = 'reconstruct3d '
    var left_image = 'c1.JPG '
    var right_image = 'c2.JPG '
    var object_name = 'obj '

    var execBtn = document.getElementById('reconstruct');
    console.log(execBtn);

    execBtn.addEventListener('click', function(ev) {
        reconstructCommand += left_image + right_image + object_name;
        console.log(reconstructCommand);
        ChildProcess.exec('touch abc');
    })
}

reconstructStart();
