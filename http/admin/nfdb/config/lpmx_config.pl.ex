#---------------------------------
  $init{DefaultLang} = 'English';          # default language

  #--- Primary server ------
  $init{prim_dbhost} = 'server1.net.com';  # primary database server
  $init{prim_dbname} = 'database1';        # primary database name
  $init{prim_dbuser} = 'user1';            # username
  $init{prim_dbpassword} = 'pwd1';         # password

  #--- Secondary server ------
  $init{sec_dbhost} = 'server2.net.com';   # secondary database server
  $init{sec_dbname} = 'database2';         # secondary database name
  $init{sec_dbuser} = 'user2';             # username
  $init{sec_dbpassword} = 'pwd2';          # password

  $init{LIST_PAGE} = 10;                   # сколько строчек показывать на одной странице для листалки страниц
  $init{LIST_LAST_USERS} = 7;              # сколько показывать недавно зарегистрированных клиентов на странице редактирования

  $init{GENPASS_MIN} = 8;                  # минимальная длина пароля для автоматической генерилки пароля
  $init{GENPASS_FLUCT} = 3;                # разброс длины пароля от $init{GENPASS_MIN} до $init{GENPASS_MIN}+$init{GENPASS_FLUCT} для автоматической генерилки пароля
#---------------------------------
1;
