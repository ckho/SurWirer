// Ionic Starter App

// angular.module is a global place for creating, registering and retrieving Angular modules
// 'starter' is the name of this angular module example (also set in a <body> attribute in index.html)
// the 2nd parameter is an array of 'requires'
// 'starter.controllers' is found in controllers.js
var easyble = evothings.easyble;
angular.module('surwirer', ['ionic', 'surwirer.controllers'])

.run(function($ionicPlatform) {
  $ionicPlatform.ready(function() {
    // Hide the accessory bar by default (remove this to show the accessory bar above the keyboard
    // for form inputs)
    if (window.cordova && window.cordova.plugins.Keyboard) {
      cordova.plugins.Keyboard.hideKeyboardAccessoryBar(true);
    }
    if (window.StatusBar) {
      // org.apache.cordova.statusbar required
      StatusBar.styleDefault();
    }
  });
})

.config(function($ionicConfigProvider, $stateProvider, $urlRouterProvider) { 
  $ionicConfigProvider.views.maxCache(0); 
  $stateProvider

  .state('app', {
    url: "/app",
    abstract: true,
    templateUrl: "templates/menu.html",
    controller: 'AppCtrl'
  })

  .state('app.measure', {
    url: "/measure",
    views: {
      'menuContent': {
        templateUrl: "templates/measure.html",
        controller: 'MeasureCtrl'
      }
    }
  })
    .state('app.measure_process', {
    url: "/measure_process",
    views: {
      'menuContent': {
        templateUrl: "templates/measure_process.html",
        controller: 'MeasureProcessCtrl'
      }
    }
  })

  .state('app.measure_result', {
    url: "/measure_result",
    views: {
      'menuContent': {
        templateUrl: "templates/measure_result.html",
        controller: 'MeasureResultCtrl'
      }
    }
  })
  .state('app.setting', {
    url: "/setting",
    views: {
      'menuContent': {
        templateUrl: "templates/setting.html",
        controller: 'ModeCtrl'
      }
    }
  })
  .state('app.history', {
    url: "/history",
    views: {
      'menuContent': {
        templateUrl: "templates/history.html"
      }
    }
  })
  .state('app.doctor', {
     url: "/doctor",
     views: {
       'menuContent': {
         templateUrl: "templates/doctor.html",
         controller: 'PlaylistsCtrl'
       }
     }
   })
  .state('app.single', {
    url: "/playlists/:playlistId",
    views: {
      'menuContent': {
        templateUrl: "templates/playlist.html",
        controller: 'PlaylistCtrl'
      }
    }
  });
  // if none of the above states are matched, use this as the fallback
  $urlRouterProvider.otherwise('/app/measure');
});
