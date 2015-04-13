

angular.module('surwirer.controllers', [])

.controller('AppCtrl', function($scope, $ionicModal, $timeout) {
  // Form data for the login modal
  $scope.mode = 0x02;
  $scope.loginData = {};
  $scope.heartrate = 0;
  // Create the login modal that we will use later
  $ionicModal.fromTemplateUrl('templates/login.html', {
    scope: $scope
  }).then(function(modal) {
    $scope.loginmodal = modal;
  });

  // Triggered in the login modal to close it
  $scope.closeLogin = function() {
    $scope.loginmodal.hide();
  };

  // Open the login modal
  $scope.login = function() {
    $scope.loginmodal.show();
  };

  // Perform the login action when the user submits the login form
  $scope.doLogin = function() {
    console.log('Doing login', $scope.loginData);

    // Simulate a login delay. Remove this and replace with your login
    // code if using a login system
    $timeout(function() {
      $scope.closeLogin();
    }, 1000);
  };
})

.controller('PlaylistsCtrl', function($scope) {
  $scope.playlists = [
    { title: 'Reggae', id: 1 },
    { title: 'Chill', id: 2 },
    { title: 'Dubstep', id: 3 },
    { title: 'Indie', id: 4 },
    { title: 'Rap', id: 5 },
    { title: 'Cowbell', id: 6 }
  ];
})

.controller('MeasureResultCtrl', function($scope) {
  //$scope.heartrate = 100;
})

.controller('MeasureCtrl', function($scope,$state,$ionicHistory) {
  //$scope.heartrate = 100;
  $scope.measure = function() {
    $ionicHistory.nextViewOptions({
      disableAnimate: true,
      disableBack: true
    });
    $state.go('app.measure_process');
  };
})

.controller('ModeCtrl', function($scope) {
  $scope.modeList = [
    { text: "Finger", value: 0x02 },
    { text: "Wrist", value: 0x03 }
  ];

  $scope.data = {
    mode: 0x02
  };
})

.controller('MeasureProcessCtrl', function($scope,$state,$ionicHistory) {
  $scope.processingHR = '--';
  $scope.debugdata = '123';
  $scope.HRarray = [];
  $scope.$on("$ionicView.enter", function() {
    startScan();
    //console.log('test00');
    //$scope.debugdata = 'test00';
  });
  startScan = function() {
    //console.log('test');
    $scope.debugdata = 'test01';
    easyble.startScan(
      function(device) {
        if (deviceIsSensorTag(device)){
          $scope.debugdata = 'Status: Device found: ' + device.name + '.';
          easyble.stopScan();
          connectToDevice(device);
          //app.stopConnectTimer()
        }
      },
      function(errorCode) {
        console.log('Error: startScan: ' + errorCode + '.');
        $scope.debugdata = 'Error: startScan: ' + errorCode + '.';
        //app.reset();
      });
  };
  deviceIsSensorTag = function(device) {
    return (device != null) &&
      (device.name != null) &&
      (device.name.indexOf('SurWirer') > -1);
  };
  connectToDevice = function(device) {
    console.log('Connecting...');
    device.connect(
      function(device) {
        console.log('Status: Connected - reading services...');
        $scope.debugdata = 'Status: Connected - reading services...';
        readServices(device);
      },
      function(errorCode) {
        console.log('Error: Connection failed: ' + errorCode + '.');
        $scope.debugdata = 'Error: Connection failed: ' + errorCode + '.';
        evothings.ble.reset();
      });
  };
  readServices = function(device) {
    device.readServices(
      ['0000180d-0000-1000-8000-00805f9b34fb'], //TO BE CONFIRMED
      // Function that monitors accelerometer data.
      startHeartRateNotification,
      function(errorCode) {
        console.log('Error: Failed to read services: ' + errorCode + '.');
        $scope.debugdata = 'Error: Failed to read services: ' + errorCode + '.';
      });
  };
  startHeartRateNotification = function(device) {
    console.log('Status: Starting heart rate notification...');
    $scope.debugdata = 'Status: Starting heart rate notification...';
    device.enableNotification(
      '00002a37-0000-1000-8000-00805f9b34fb', //TO BE CONFIRMED
      function(data,$ionicHistory,$state) {
        //console.log('byteLength: '+data.byteLength);
        var dataArray = new Int8Array(data);
        //console.log('length: '+dataArray.length);
        //console.log('data: '+dataArray[0]+' '+dataArray[1]+' '+dataArray[2]);
        $scope.processingHR = dataArray[1];
        //if ($scope.processingHR > 0) $scope.HRarray.push($scope.processingHR);
        //if($scope.HRarray.size >= 2) {
          if ((dataArray[0]!=0)||(dataArray[1]!=0)){
            easyble.stopScan();
            easyble.closeConnectedDevices();
            $scope.debugdata = 'Done!';
            /*$rootScope.heartrate = $scope.processingHR;
            $ionicHistory.nextViewOptions({
              disableBack: true
           });
            $state.go('app.measure_result');*/
          }
          //for(var x = 0; x < $scope.HRarray.size; x ++) {
            //Sum += $scope.HRarray[x];
          //}
          
        //}
      },
      function(errorCode) {
        console.log('Error: enableNotification: ' + errorCode + '.');
        $scope.debugdata = 'Error: enableNotification: ' + errorCode + '.';
      });
  };
});