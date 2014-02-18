//Script functions to show dialogs

function textDialog (control, title, prompt) {
	var dialog = control.addChild("(@ui)/textDialog.xml");
	
	dialog.content.title.text = title;
	dialog.content.prompt.text = prompt;
	
	dialog.grabEvents();
};
