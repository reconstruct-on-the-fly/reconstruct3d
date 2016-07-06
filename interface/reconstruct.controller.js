angular.module('ReconstructApp', ['ngMaterial'])
    .controller('formCtrl', function($scope) {

    var vm = this;

    angular.extend(vm, {
        command: '../reconstruct3d ',
        image: {
            left: '',
            right: ''
        },
        disparity: false,
        disp: {
            min: '',
            max: '',
            size: '',
            wls: {
                filter: false,
                lambda: '',
                sigma:''
            },
            noise: {
                filter: false,
                size: '',
                threshold: ''
            }
        },
        reconstruct: false,
        obj: {
            height: '',
            simplify: '',
            laplacescale: '',
            laplaceiterations: ''
        },
        project_title: '',
        rectify: false,
        imagePair: true,
        printCommand: printCommand,
        isComplete: isComplete,
    });

    activate();


    //////
    function activate() {
    }

    function executeCommand (command) {
        const ChildProcess = require('child_process');
        ChildProcess.exec('echo "' + command +'" > log.txt');
        ChildProcess.exec(command + ' >> log.txt');
    }

    function isComplete() {
        return vm.image.left !== '' && vm.project_title !== '' &&
               ((vm.image.right !== '' && vm.imagePair) ||
                (!vm.imagePair));
    }

    function printCommand() {
        if(vm.imagePair)
            vm.command += ' --image-pair ' + vm.image.left + ' ' +  vm.image.right;
        else
            vm.command += ' --set ' + vm.image.left;

        vm.command += ' --project-title ' + vm.project_title;

        if(vm.disparity === false) {
            vm.command += ' --no-disparity';
        }
        if(vm.reconstruct === false) {
            vm.command += ' --no-reconstruction ';
        }
        if(vm.disp.wls.filter === false) {
            vm.command += ' --no-wls-filter ';
        }
        if(vm.disp.noise.filter === false) {
            vm.command += ' --no-noise-reduction-filter ';
        }
        if(vm.rectify === true) {
            vm.command += ' --with-rectification ';
        }

        vm.command += ' --disparity-range ' + vm.disp.min + ' ' + vm.disp.max;
        vm.command += ' --disparity-window ' + vm.disp.size;
        vm.command += ' --wls-filter ' + vm.disp.wls.lambda + ' ' + vm.disp.wls.sigma;
        vm.command += ' --noise-reduction-filter ' + vm.disp.noise.size + ' ' + vm.disp.noise.threshold;

        vm.command += ' --obj-height ' + vm.obj.height;
        vm.command += ' --laplace-scale ' + vm.obj.laplacescale;
        vm.command += ' --laplace-iterations ' + vm.obj.laplaceiterations;
        vm.command += ' --obj-simplify ' + vm.obj.simplify;

        console.log(vm.command);

        executeCommand(vm.command);
    }

});
