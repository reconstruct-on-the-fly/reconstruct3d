angular.module('ReconstructApp', ['ngMaterial'])
    .controller('formCtrl', function($scope) {

    var vm = this;

    angular.extend(vm, {
        command: 'reconstruct3d ',
        image: {
            left: '',
            right: ''
        },
        disparity: false,
        disp: {
            min: 1,
            max: 2,
            size: 1,
            wls: {
                filter: false,
                lambda: 1,
                sigma:1
            },
            noise: {
                filter: false,
                size: 1,
                threshold: 1
            }
        },
        reconstruct: false,
        obj: {
            height: 1,
            simplify: 0.1,
            laplacescale: 0.1,
            laplaceiterations: 2
        },

        printCommand: printCommand
    });

    activate();

    //////
    function activate() {
        console.log('--');
    }

    function printCommand() {
        vm.command += ' ' + vm.image.left + ' ' +  vm.image.right;

        if(vm.disparity === false) {
            vm.command += ' --no-disparity ';
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

        vm.command += ' --disparity-range ' + vm.disp.min + ' ' + vm.disp.max;
        vm.command += ' --disparity-window ' + vm.disp.size;
        vm.command += ' --wls-filter ' + vm.disp.wls.lambda + ' ' + vm.disp.wls.sigma;
        vm.command += ' --noise-reduction-filter ' + vm.disp.noise.size + ' ' + vm.disp.noise.threshold;

        vm.command += ' --obj-height ' + vm.obj.height;
        vm.command += ' --laplace-scale ' + vm.obj.laplacescale;
        vm.command += ' --laplacce-iterations ' + vm.obj.laplaceiterations;
        vm.command += ' --obj-simplify ' + vm.obj.simplify;

        console.log(vm.command);
    }

});
