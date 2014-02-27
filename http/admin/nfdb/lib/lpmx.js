
function FormOnSubmit(formID)
{
   formID.submit();
}

function paste(ip,name,p,s,w,formID){
    formID.ip.value = ip;
    formID.name.value = name;
    formID.p.value = p;
    formID.s.value = s;
    formID.w.value = w;
    window.scroll(0,0);
}


function SetDeleteMark(formID,dict_delete,dict_edit)
{
  // delete mark
  if(formID.unid.value != ''){
      if(formID.delflag.checked){
          formID.SaveCmd.value = dict_delete;
      }else{
          formID.SaveCmd.value = dict_edit;
      }
  }else{
      formID.delflag.checked = false;
  }
}

function open_child(scriptname,w,h) {
  var newPath;
  newPath=scriptname;
  ca_window = open(newPath, "MyWin","resizable=yes,scrollbars=yes,width="+w+",height="+h+",top=50,left=150");
  ca_window.focus();
  return false;
}

function close_child() {
  if (ca_window != null && !ca_window.closed) ca_window.close();
  return false;
}
