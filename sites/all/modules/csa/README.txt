
This file contains last minute or needed information for installing and running
CSA.

When replace CSA 1.0.x or lower with CSA 1.1.0 of higher note;
- Updated CSA to latest version of SDK. When upgrading to 1.1+, make sure you
  disable all CSA modules and the MediaMosa connector module. Then upgrade the
  code; best is to replace the whole code tree (re-use the settings.php). What
  you need to know is that mediamosa_connector module has been moved. Drupal has
  no real method to fix this by update scripts. So its very important that you
  turn off the CSA module and mediamosa_connector module and then replace the
  code. Do not copy over the current version(!).



Optional;
Install https using securepages/securepages_prevent_hijack

1)
Create a default https certificate (for testing only)
sudo make-ssl-cert /usr/share/ssl-cert/ssleay.cnf /your-path-to-certification/csa.crt --force-overwrite

2)
Apache settings
<VirtualHost csa.local:443>
  ServerName csa.local
  DocumentRoot /your-path-to-code/csa

  SSLEngine on
  SSLCertificateFile \
    /your-path-to-certification/csa.crt
  SSLCertificateKeyFile \
    /your-path-to-certification/csa.crt

  <Directory /your-path-to-code/csa>
   Options Indexes FollowSymLinks MultiViews
   AllowOverride All
    Order deny,allow
    Allow from All
  </Directory>

  LogLevel warn
  ErrorLog /var/log/apache2/csa_error.log
  CustomLog /var/log/apache2/csa_access.log combined
</VirtualHost>

3)
Enable mod ssl in apache;
sudo a2enmod ssl

4)
To .htaccess file in root of installation add;
# HTTPS.
RewriteCond %{SERVER_PORT} 80
RewriteRule ^(.*)$ https://%{HTTP_HOST}/$1 [R,L]

5)
settings.php add;
if (!empty($_SERVER['HTTPS'])) {
  ini_set('session.cookie_secure', 1);
  $base_url = 'https://csa.local';
}
else {
  $base_url = 'http://csa.local';
}
