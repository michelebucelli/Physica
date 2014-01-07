//Easing functions

//Linear easing
var easings.linear = function (t) {	return t; };



//Quadratic easings
var easings.quadraticIn = function (t) { return Math.pow(t, 2); };

var easings.quadraticOut = function (t) { return 1 - Math.pow(t - 1, 2); };

var easings.quadraticInOut = function (t) {
	if (t < 0.5) return 2 * Math.pow(t, 2);
	else return 1 - Math.pow(2 * t - 2, 2) / 2;
};



//Cubic easings
var easings.cubicIn = function (t) { return Math.pow(t, 3); };

var easings.cubicOut = function (t) { return 1 + Math.pow(t - 1, 3); };

var easings.cubicInOut = function (t) {
	if (t < 0.5) return 4 * Math.pow(t, 3);
	else return 1 + Math.pow(2 * t - 2, 3) / 2;
};



//Quartic easings
var easings.quarticIn = function (t) { return Math.pow(t, 4); };

var easings.quarticOut = function(t) { return 1 - Math.pow(t - 1, 4); };

var easings.quarticInOut = function (t) {
	if (t < 0.5) return 8 * Math.pow(t, 4);
	else return 1 - Math.pow(2 * t - 2, 4) / 2;
};



//Quintic easings
var easings.quinticIn = function (t) { return Math.pow(t, 5); };

var easings.quinticOut = function (t) { return 1 + Math.pow(t - 1, 5); };

var easings.quinticInOut = function (t) {
	if (t < 0.5) return 16 * Math.pow(t, 5);
	else return 1 - Math.pow(2 * t - 2, 5) / 2;
};



//Algebric general easing
//Generalise the above ones
var easings.algebricIn = function (t, n) { return Math.pow(t, n); };

var easings.algebricOut = function (t, n) { return 1 + Math.pow(-1, n - 1) * Math.pow(t - 1, n); };

var easings.algebricInOut = function (t, n) {
	if (t < 0.5) return Math.pow(2, n - 1) * Math.pow(t, n);
	else return 1 + Math.pow(-1, n - 1) * Math.pow(2 * t - 2, n) / 2;
};



//Sinusoidal easings
var easings.sineIn = function (t) { return 1 - Math.cos(t * Math.PI() / 2); };

var easings.sineOut = function (t) { return Math.sin(t * Math.PI() / 2); };

var easings.sineInOut = function (t) { return (1 - Math.cos(Math.PI() * t)) / 2; };



//Exponential easings
var easings.exponentialIn = function (t) { if (t == 0) return 0; else return Math.pow(2, 10 * (t - 1)); };

var easings.exponentialOut = function (t) { if (t == 1) return 1; else return -Math.pow(2, -10 * t) + 1; };

var easings.exponentialInOut = function (t) {
	if (t == 0) return 0; 
	else if (t == 1) return 1;	
	else if (t < 0.5) return Math.pow(2, 10 * (2 * t - 1) - 1); 
	else return -Math.pow(2, -10 * (2 * t - 1) - 1) + 1;
};



//Circular easings
var easings.circularIn = function (t) { return -Math.sqrt(1 - Math.pow(t, 2)) + 1; };

var easings.circularOut = function (t) { return Math.sqrt(1 - Math.pow(t - 1, 2)); };

var easings.circularInOut = function (t) { 
	if (t < 0.5) return (1 - Math.sqrt(1 - 4 * t * t)) / 2;
	else return (Math.sqrt(1 - Math.pow(2 * t - 2, 2)) + 1) / 2;
};



//Elastic easings
var easings.elasticIn = function (t, a, p) {
	if (t == 0) return 0;
	if (t == 1) return 1;
	
	if (p == 0) p = 0.3;
	
	if (a < 1) { a = 1; var s = p / 4; }
	else var s = p / (2 * Math.PI()) * Math.asin(1 / a);
	
	return -a * Math.pow(2, 10 * (t - 1)) * Math.sin((t - 1 - s) * (2 * Math.PI()) / p);
};

var easings.elasticOut = function (t, a, p) {
	if (t == 0) return 0;
	if (t == 1) return 1;
	
	if (p == 0) p = 0.3;
	
	if (a < 1) { a = 1; var s = p / 4; }
	else var s = p / (2 * Math.PI()) * Math.asin(1 / a);
	
	return a * Math.pow(2, -10 * t) * Math.sin((t - s) * (2 * Math.PI()) / p) + 1;
};

var easings.elasticInOut = function (t, a, p) {
	if (t == 0) return 0;
	if (t == 1) return 1;
	
	if (p == 0) p = 0.45;
	
	if (a < 1) { a = 1; var s = p / 4; }
	else var s = p / (2 * Math.PI()) * Math.asin(1 / a);
	
	if (t < 0.5) var result = - (a * Math.pow(2, 10 * (2 * t - 1)) * Math.sin((2 * t - 1 - s) * (2 * Math.PI()) / p)) / 2;
	else var result = a * Math.pow(2, -10 * (2 * t - 1)) * Math.sin((2 * t - 1 - s) * (2 * Math.PI()) / p) / 2 + 1;
	
	return result;
};



//Back easings
var easings.backIn = function (t, s) {
	return t * t * ((s + 1) * t - s);
};

var easings.backOut = function (t, s) {
	return (t - 1) * (t - 1) * ((s + 1) * (t - 1) + s) + 1;
};

var easings.backInOut = function (t, s) {
	s = s * 1.525;
	
	if (t < 0.5) return 2 * t * t * (((s + 1) * (2 * t) - s));
	else return ((2 * t - 2) * (2 * t - 2) * ((s + 1) * (2 * t - 2) + s) + 2) / 2;
};



//Bounce easings
var easings.bounceIn = function (t) {
	return 1 - easings.bounceOut(1 - t);
};

var easings.bounceOut = function (t) {
	if (t < 1 / 2.75) return 7.5625 * t * t;
	else if (t < 2 / 2.75) return 7.5625 * Math.pow(t - 1.5 / 2.75, 2) + 0.75;
	else if (t < 2.5 / 2.75) return 7.5625 * Math.pow(t - 2.25 / 2.75, 2) + 0.9375;
	else return 7.5625 * Math.pow(t - 2.625 / 2.75, 2) + 0.984375;
};

var easings.bounceInOut = function (t) {
	if (t < 0.5) return easings.bounceIn(t * 2) / 2;
	else return (easings.bounceOut(t * 2 - 1) + 1) / 2;
};