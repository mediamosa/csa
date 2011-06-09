/* 
	File: handlers.js
	Note: This code is mainly based on the demo "SWFUpload Demos - Simple Demo" 
*/


/* **********************
   Event Handlers
   These are my custom event handlers to make my
   web application behave the way I went when SWFUpload
   completes different tasks.  These aren't part of the SWFUpload
   package.  They are part of my application.  Without these none
   of the actions SWFUpload makes will show up in my application.
   ********************** */

function fileQueued(file) {
	try {
		var progress = new FileProgress(file, this.customSettings.progressTarget);
		progress.setStatus(Drupal.t('Mediafile queued.'));
		progress.toggleCancel(true, this);

	} catch (ex) {
		this.debug(ex);
	}
}

function fileQueueError(file, errorCode, message) {
	try {
		if (errorCode === SWFUpload.QUEUE_ERROR.QUEUE_LIMIT_EXCEEDED) {
			alert(Drupal.t("You have attempted to queue too many files.") + "\n" + (Drupal.t("You may select @files?", { '@files': Drupal.formatPlural(message, 'one file.', 'up to @count files.')})));
			return;
		}

		var progress = new FileProgress(file, this.customSettings.progressTarget);
		progress.setError();
		progress.toggleCancel(false);

		switch (errorCode) {
		case SWFUpload.QUEUE_ERROR.FILE_EXCEEDS_SIZE_LIMIT:
			progress.setStatus(Drupal.t("File is too big."));
			this.debug("Error Code: File too big, File name: " + file.name + ", File size: " + file.size + ", Message: " + message);
			break;
		case SWFUpload.QUEUE_ERROR.ZERO_BYTE_FILE:
			progress.setStatus(Drupal.t("Cannot upload Zero Byte files."));
			this.debug("Error Code: Zero byte file, File name: " + file.name + ", File size: " + file.size + ", Message: " + message);
			break;
		case SWFUpload.QUEUE_ERROR.INVALID_FILETYPE:
			progress.setStatus(Drupal.t("Invalid File Type."));
			this.debug("Error Code: Invalid File Type, File name: " + file.name + ", File size: " + file.size + ", Message: " + message);
			break;
		default:
			if (file !== null) {
				progress.setStatus(Drupal.t("Unhandled Error"));
			}
			this.debug("Error Code: " + errorCode + ", File name: " + file.name + ", File size: " + file.size + ", Message: " + message);
			break;
		}
	} catch (ex) {
        this.debug(ex);
    }
}

function fileDialogComplete(numFilesSelected, numFilesQueued) {
	try {
		if (numFilesSelected > 0) {
			document.getElementById(this.customSettings.cancelButtonId).disabled = false;
			document.getElementById(this.customSettings.uploadButtonId).disabled = false;
		}
		
		/* I want auto start the upload and I can do that here */
		// this.startUpload();
	} catch (ex)  {
        this.debug(ex);
	}
}

function uploadStart(file) {
	try {
		/* I don't want to do any file validation or anything,  I'll just update the UI and
		return true to indicate that the upload should start.
		It's important to update the UI here because in Linux no uploadProgress events are called. The best
		we can do is say we are uploading.
		 */
		var progress = new FileProgress(file, this.customSettings.progressTarget);
		progress.setStatus(Drupal.t("Uploading..."));
		progress.toggleCancel(true, this);      
	}
	catch (ex) {
	}
	
	return true;
}

function uploadProgress(file, bytesLoaded, bytesTotal) {
	try {
		var percent = Math.ceil((bytesLoaded / bytesTotal) * 100);

		var progress = new FileProgress(file, this.customSettings.progressTarget);
		progress.setProgress(percent);
		progress.setStatus(Drupal.t("Uploading..."));
        
        // Check if there are any form errors; if true, stop everything
        stopOnFormerrors();
        
	} catch (ex) {
		this.debug(ex);
	}
}

function uploadSuccess(file, serverData) {
	try {
		var progress = new FileProgress(file, this.customSettings.progressTarget);
	    var response = Drupal.parseJson(serverData); // parse server response

		progress.setComplete();
		progress.setStatus(Drupal.t("File uploaded."));
		progress.toggleCancel(false);
		
		// Update our quota and do post stuff.
		if (!jsTimer) {
			jsTimer = window.setInterval("updateMediaMosaQuotaText()", 500);
		}
		
	} catch (ex) {
		this.debug(ex);
	}
}

function uploadError(file, errorCode, message) {
	try {
		var progress = new FileProgress(file, this.customSettings.progressTarget);
		progress.setError();
		progress.toggleCancel(false);

		switch (errorCode) {
		case SWFUpload.UPLOAD_ERROR.HTTP_ERROR:
			progress.setStatus(Drupal.t('Upload Error: %message', { '%message': message }));
			this.debug("Error Code: HTTP Error, File name: " + file.name + ", Message: " + message);
			break;
		case SWFUpload.UPLOAD_ERROR.UPLOAD_FAILED:
			progress.setStatus(Drupal.t("Upload Failed."));
			this.debug("Error Code: Upload Failed, File name: " + file.name + ", File size: " + file.size + ", Message: " + message);
			break;
		case SWFUpload.UPLOAD_ERROR.IO_ERROR:
			progress.setStatus(Drupal.t("Server (IO) Error"));
			this.debug("Error Code: IO Error, File name: " + file.name + ", Message: " + message);
			break;
		case SWFUpload.UPLOAD_ERROR.SECURITY_ERROR:
			progress.setStatus(Drupal.t("Security Error"));
			this.debug("Error Code: Security Error, File name: " + file.name + ", Message: " + message);
			break;
		case SWFUpload.UPLOAD_ERROR.UPLOAD_LIMIT_EXCEEDED:
			progress.setStatus(Drupal.t("Upload limit exceeded."));
			this.debug("Error Code: Upload Limit Exceeded, File name: " + file.name + ", File size: " + file.size + ", Message: " + message);
			break;
		case SWFUpload.UPLOAD_ERROR.FILE_VALIDATION_FAILED:
			progress.setStatus(Drupal.t("Failed Validation. Upload skipped."));
			this.debug("Error Code: File Validation Failed, File name: " + file.name + ", File size: " + file.size + ", Message: " + message);
			break;
		case SWFUpload.UPLOAD_ERROR.FILE_CANCELLED:
			// If there aren't any files left (they were all cancelled) disable the cancel button
			if (this.getStats().files_queued === 0) {
				document.getElementById(this.customSettings.cancelButtonId).disabled = true;
				document.getElementById(this.customSettings.uploadButtonId).disabled = true;
			}
			progress.setStatus(Drupal.t("Cancelled"));
			progress.setCancelled();
            window.clearInterval(jsTimer);
            jsTimer = false;
			break;
		case SWFUpload.UPLOAD_ERROR.UPLOAD_STOPPED:
			progress.setStatus(Drupal.t("Stopped"));
            window.clearInterval(jsTimer);
            jsTimer = false;
			break;
		default:
			progress.setStatus(Drupal.t("Unhandled Error: %code", {'%code' : errorCode}));
			this.debug("Error Code: " + errorCode + ", File name: " + file.name + ", File size: " + file.size + ", Message: " + message);
			break;
		}
	} catch (ex) {
        this.debug(ex);
    }
}

function uploadComplete(file) {
	if (this.getStats().files_queued === 0) {
		document.getElementById(this.customSettings.cancelButtonId).disabled = true;
		document.getElementById(this.customSettings.uploadButtonId).disabled = true;
	}
}

// This event comes from the Queue Plugin
function queueComplete(numFilesUploaded) {
	
	var status = document.getElementById("divStatus");
	numFilesUploaded = numFilesUploaded - count_failed_uploads; // get real number of successful uploads
	status.innerHTML = Drupal.formatPlural(numFilesUploaded, '1 file uploaded, visit the !link to continue with metadata.', '@count files uploaded, visit the !link to continue with metadata.', { '!link':'<a href="/csa/unpublished">unpublished page</a>'});

	// fire up our function --> upload complete
	queue_complete = numFilesUploaded;   
}

function updateMediaMosaQuotaText() {
	// Make sure it doesn't trigger anymore.
	window.clearInterval(jsTimer);

	try {
    	// Update the quota text.
    	var csaquota = document.getElementById("csa_quota");
    	if (csaquota) {
    		var result_text = [];
    		
    		$.ajax({
    			url: Drupal.settings.basePath + 'csa/upload/updatequota',
    			async: false,
    			dataType: 'json',
    			success: function (data) {
    				result_text = data.text;
    			}
    		});

    		// Set the text.
    		csaquota.innerHTML = result_text;
    	}
    } catch (ex) {
        // Doing nothing
    }    

    // Can add again.
	jsTimer = false;
}

function stopOnFormerrors() {
    if (isNaN(document.getElementById('form_errors'))) {
        var formerrors = parseInt(document.getElementById('form_errors').value);
        if (formerrors == 1 && error_send == false) {
            // form error --> kill everything
            error_send = true;
            window.clearInterval(jsTimer);
            jsTimer = false;
            swfu.cancelQueue();
            document.getElementById('edit-delete-queue').click();
            
            // restore old form elements            
            setTimeout("upload_complete = false;swfu.setButtonDisabled(false);document.getElementById('startuploadbutton').value = Drupal.t('Save');document.getElementById('startuploadbutton').onclick = function() {startUploadProcess();window.location.href='#uploadform';};", 1000);
            setTimeout("document.getElementById('imagepreviewlistbutton').style.visibility = 'hidden';document.getElementById('divStatus').innerHTML = Drupal.t('Upload failed.'); ", 1000);                   
        }
        if (formerrors == 2 && error_send == false) {
          // error during upload (e.x. node file restriction)
          error_send = true;
          window.clearInterval(jsTimer);
          jsTimer = false;
          swfu.cancelQueue();
          document.getElementById('edit-delete-queue').click();
        }
    }
}
