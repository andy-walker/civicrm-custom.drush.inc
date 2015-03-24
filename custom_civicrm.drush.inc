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

function drush_custom_civicrm_install() {

    # just test stdout / stderr for now
    for ($i=0;$i<10;$i++) {
        if (mt_rand(0, 50) == 15)
            return drush_log('Oh dear, an error.', 'error');
        drush_log('Everything is ok', 'ok');
    }

}

function drush_custom_civicrm_upgrade() {
    return drush_custom_civicrm_install();
}