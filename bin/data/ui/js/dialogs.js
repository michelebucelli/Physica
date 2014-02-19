//Script functions to show dialogs

//Shows a text dialog
function textDialog (control, title, prompt, defaultText, after) {
	var dialog = control.addChild("(@ui)/textDialog.xml");
	
	dialog.content.title.text = title;
	dialog.content.prompt.text = prompt;
	dialog.field.content.text.text = defaultText;
	dialog.field.userDefined.storedText = defaultText;
	
	dialog.userDefined.after = after;
	
	dialog.grabEvents();
};

//Shows an image dialog
function imageDialog (control, title, defaultImg, after) {
	var dialog = control.addChild("(@ui)/imageDialog.xml");
	
	dialog.content.title.text = title;
	dialog.userDefined.after = after;
	
	dialog.grabEvents();
}

//Returns to the caller
function dialogReturn (dialog, result) {
	dialog.releaseEvents();
	dialog.parent.removeChild(dialog.id);
	
	dialog.parent.userDefined.dialogResult = result;
	
	if (dialog.userDefined.after != "")
		dialog.parent.scriptTimeout ( 0, dialog.userDefined.after );
}
