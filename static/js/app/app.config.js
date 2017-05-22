(function(){
    'use strict';
    angular.module('app').config(function($interpolateProvider) {
    $interpolateProvider.startSymbol('({').endSymbol('})');
  });
})();