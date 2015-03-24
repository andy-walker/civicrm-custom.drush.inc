<?php


function custom_civicrm_drush_command() {

    $items['custom-civicrm-install'] = array(
        'callback'    => 'drush_custom_civicrm_install',
        'description' => "Perform custom CiviCRM installation"
    );

    $items['custom-civicrm-upgrade'] = array(
        'callback'    => 'drush_custom_civicrm_upgrade',
        'description' => "Perform custom CiviCRM upgrade"
    );

    return $items;

}

function civicrm_custom_drush_help($section) {
    
    switch ($section) {

        case 'drush:custom-civicrm-install':
            return dt("Circle customized version of civicrm-install");
    
        case 'drush:custom-civicrm-upgrade':
            return dt("Circle customized version of civicrm-upgrade");

    }

}

function drush_custom_civicrm_install_validate() {
    
    if (drush_get_option('db-url', FALSE)) {
        $db_spec['db-url'] = $GLOBALS['db_url'];
    } elseif (drush_get_option('all', FALSE)) {
        $db_spec = _drush_sql_get_all_db_specs();
    }

    if (!isset($db_spec)) {
        $db_spec = drush_civicrm_get_db_spec();
    }

    if (!drush_get_option('dbuser', FALSE)) {
        drush_set_option('dbuser', $db_spec['username']);
    }

    if (!drush_get_option('dbpass', FALSE)) {
        drush_set_option('dbpass', $db_spec['password']);
    }

    if (!drush_get_option('dbhost', FALSE)) {
        drush_set_option('dbhost', $db_spec['host']);
    }

    if (!drush_get_option('dbname', FALSE)) {
        drush_set_option('dbname', $db_spec['database']);
    }
    
    $crmpath    = _civicrm_get_crmpath();
    $drupalRoot = drush_get_context('DRUSH_DRUPAL_ROOT');
    $modPath    = "$drupalRoot/$crmpath";

    if (drush_get_option('lang', FALSE) && !drush_get_option('langtarfile', FALSE)) {
        return drush_set_error('CIVICRM_INSTALL_LANGTARFILE_NOT_SPECIFIED', dt('CiviCRM language tarfile not specified.'));
    }

    return TRUE;

}

function drush_custom_civicrm_install() {

    $dbuser = drush_get_option('dbuser', FALSE);
    $dbpass = drush_get_option('dbpass', FALSE);
    $dbhost = drush_get_option('dbhost', FALSE);
    $dbname = drush_get_option('dbname', FALSE);
    
    $crmpath    = _civicrm_get_crmpath();
    $drupalRoot = drush_get_context('DRUSH_DRUPAL_ROOT');
    $modPath    = "$drupalRoot/$crmpath";
    $lang       = drush_get_option('lang', '');

    if (!is_dir("$modPath/civicrm")) {
        drush_log("No installation directory found at $modPath/civicrm", 'error');
    }

    // include civicrm installer helper file
    $civicrmInstallerHelper = "$modPath/civicrm/install/civicrm.php";
    if (!file_exists($civicrmInstallerHelper)) {
        return drush_set_error('CIVICRM_NOT_PRESENT', dt("CiviCRM installer helper file is missing."));
    }

    if ($lang != '') {
        _civicrm_extract_tarfile($modPath, "langtarfile");
    }
    
    // setup all required files/civicrm/* directories
    if (!_civicrm_create_files_dirs($civicrmInstallerHelper, $modPath)) {
        return FALSE;
    }

    // install database
    _civicrm_install_db($dbuser, $dbpass, $dbhost, $dbname, $modPath, $lang);
    
    // generate civicrm.settings.php file
    _civicrm_generate_settings_file($dbuser, $dbpass, $dbhost, $dbname, $modPath);
    module_enable(array('civicrm'));
    drush_log(dt("CiviCRM installed."), 'ok');

}

/**
* Implementation of drush_hook_COMMAND_validate for command 'custom-civicrm-upgrade'
*/
function drush_custom_civicrm_upgrade_validate() {

    if (!defined('CIVICRM_UPGRADE_ACTIVE')) {
        define('CIVICRM_UPGRADE_ACTIVE', 1);
    }
    
    return _civicrm_init();

}

function drush_custom_civicrm_upgrade() {
    
    global $civicrm_root;

    $tarfile     = drush_get_option('tarfile', FALSE);
    $date        = date('YmdHis');
    $backup_file = drush_get_option('uri', "civicrm");
    $basepath    = explode('/', $civicrm_root);
    
    array_pop($basepath);

    $project_path = implode('/', $basepath) . '/';
    $drupal_root  = drush_get_context('DRUSH_DRUPAL_ROOT');
    $backup_dir   = drush_get_option('backup-dir', $drupal_root . '/backup');
    $backup_dir   = rtrim($backup_dir, '/');
    
    drush_print(dt("\nThe upgrade process involves - "));

    drush_print(dt("1. Backing up database as => !path",
        array('!path' => "$backup_dir/modules/$date/$backup_file.sql")
    ));

    drush_print(dt("2. Executing civicrm/upgrade?reset=1 just as a browser would.\n"));
    
    if (!drush_confirm(dt('Do you really want to continue?')))
        return drush_user_abort();

    if (!is_writable($backup_dir))
        return drush_log("$backup_dir is not writable");

    @drush_op('mkdir', $backup_dir, 0777);
    $backup_dir .= '/modules';
    @drush_op('mkdir', $backup_dir, 0777);
    $backup_dir .= "/$date";
    @drush_op('mkdir', $backup_dir, 0777);
    $backup_target = $backup_dir . '/' . $backup_file;

    drush_set_option('result-file', $backup_target . '.sql');
    drush_civicrm_sqldump();

    drush_log(dt('1. Database backed up.'), 'ok');
    drush_log(dt("2. "));
    
    if (drush_civicrm_upgrade_db_validate())
        drush_civicrm_upgrade_db();

    drush_log(dt("\nProcess completed."), 'completed');

}