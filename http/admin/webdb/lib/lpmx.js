function SetCloseTime(formID,cur_date,cur_time)
{
   // close 
   if(formID.action.value == 2){
      formID.dclose.value = cur_date + ' ' + cur_time
   }else{
      formID.dclose.value = ''
   }
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
