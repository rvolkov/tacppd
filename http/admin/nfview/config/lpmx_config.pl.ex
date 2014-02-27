#---------------------------------

    $init{DefaultLang} = 'English';       # язык по умолчанию

    #--- Primary server ------
    $init{prim_dbhost} = 'db.host.com';  # сервер базы данных
    $init{prim_dbname} = 'lpmx';               # имя базы данных
    $init{prim_dbuser} = 'web_lpmx';           # логин для доступа к базе
    $init{prim_dbpassword} = 'lpmx';           # пароль для доступа к базе


    $init{LIST_PAGE} = 10;                # сколько строчек показывать на одной странице для листалки страниц
    $init{LIST_LAST_USERS} = 7;           # сколько показывать недавно зарегистрированных клиентов на странице редактирования

    $init{GENPASS_MIN} = 8;               # минимальная длина пароля для автоматической генерилки пароля
    $init{GENPASS_FLUCT} = 3;             # разброс длины пароля от $init{GENPASS_MIN} до $init{GENPASS_MIN}+$init{GENPASS_FLUCT} для автоматической генерилки пароля


#---------------------------------

1; 
