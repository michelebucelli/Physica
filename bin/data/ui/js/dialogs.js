//Script functions to show dialogs

//Shows a text dialog
function textDialog (control, title, prompt, after) {
	var dialog = control.addChild("(@ui)/textDialog.xml");
	
	dialog.content.title.text = title;
	dialog.content.prompt.text = prompt;
	
	dialog.userDefined.after = after;
	
	dialog.grabEvents();
};

//Returns to the caller
function dialogReturn (dialog, result) {
	dialog.releaseEvents();
	dialog.parent.removeChild(dialog.id);
	
	dialog.parent.userDefined.dialogResult = result;
	dialog.parent.scriptTimeout ( 0, dialog.userDefined.after );
}
