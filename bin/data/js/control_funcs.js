//Script functions related to controls

importScript("(#easings)");

var menu_changeTime = 350;
var menu_changeEasing_show = easings.quarticInOut;
var menu_changeEasing_hide = easings.quarticInOut;

var comboBox_easingOpen = easings.backOut;
var comboBox_easingClose = easings.backIn;

var fillbar_easingFill = easings.elasticOut;

function menu_show (menu, direction) {
	menu.area.y = direction * viewport.h;
	menu.area.setEasingY(direction * viewport.h, -direction * viewport.h, menu_changeTime, menu_changeEasing_show, 1.5, 0);
	menu.status = status_normal;
	menu.visible = 1;
}

function menu_hide (menu, direction) {
	menu.area.y = 0;
	menu.area.setEasingY(0, direction * viewport.h, menu_changeTime, menu_changeEasing_hide, 1.5, 0);
	menu.status = status_disabled;
}

function comboBox_open (comboBox) {
	comboBox.selected.status = status_active;
	comboBox.grabEvents();
	
	var comboBox.userDefined.open = 1;
	
	var counter = 0;
	var child;
	
	for (child = comboBox.firstChild; child != 0; child = child.nextSibling){
		if (child.id != "selected"){
			child.area.setEasingY(0, 2 + comboBox.area.h + counter * child.area.h, 250, comboBox_easingOpen, 3, 0);
			child.status = status_normal;
			
			counter++;
		}
	}
}

function comboBox_close (comboBox) {
	comboBox.selected.status = status_normal;
	comboBox.releaseEvents();
	
	var comboBox.userDefined.open = 0;
		
	var counter = 0;
	var child;
	
	for (child = comboBox.firstChild; child != 0; child = child.nextSibling){
		if (child.id != "selected"){
			child.area.setEasingY(child.area.y, -child.area.y, 250, comboBox_easingClose, 3, 0);
			child.status = status_disabled;
				
			counter++;
		}
	}
}

function fillbar_setFill(fillbar, fill){
	var w = fill * (fillbar.area.w - 8);
	if (w < 16) w = 16;
	
	fillbar.userDefined.fill = fill;	
	fillbar.fill.area.setEasingW(fillbar.fill.area.w, w - fillbar.fill.area.w, 750, fillbar_easingFill, 0, 0);
}