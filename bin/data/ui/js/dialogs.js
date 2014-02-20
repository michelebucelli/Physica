//Script functions to show dialogs

//Shows a text dialog
function textDialog (control, title, prompt, defaultText, after) {
	var dialog = control.root.addChild("(@ui)/textDialog.xml");
	
	dialog.content.title.text = title;
	dialog.content.prompt.text = prompt;
	dialog.field.content.text.text = defaultText;
	dialog.field.userDefined.storedText = defaultText;
			
	dialog.userDefined.after = after;
	dialog.userDefined.owner = control.completeId;
	
	dialog.grabEvents();
	
	return dialog;
};

//Shows an image dialog
function imageDialog (control, title, defaultImg, after) {
	var dialog = control.root.addChild("(@ui)/imageDialog.xml");
	
	dialog.content.title.text = title;	
	dialog.userDefined.after = after;
	dialog.userDefined.owner = control.completeId;
	
	dialog.grabEvents();
	
	return dialog;
}

//Shows an image viewer dialog
function imageViewerDialog (control, img, after) {
	var dialog = control.root.addChild("(@ui)/imageViewerDialog.xml");
	
	dialog.content.image.image = img;	
	dialog.userDefined.after = after;
	dialog.userDefined.owner = control.completeId;
	
	dialog.grabEvents();
	
	return dialog;
}

//Returns to the caller
function dialogReturn (dialog, result) {
	dialog.releaseEvents();
	dialog.parent.removeChild(dialog.id);
	
	var c = dialog.parent.child(dialog.userDefined.owner);
	output("DIALOG RETURNING TO " + dialog.userDefined.owner + " (" + c.id + ")");
	
	c.userDefined.dialogResult = result;
	
	if (dialog.userDefined.after != "")
		c.scriptTimeout ( 0, dialog.userDefined.after );
}
