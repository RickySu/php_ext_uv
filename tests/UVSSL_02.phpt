--TEST--
Check for UVSSL Connect
--FILE--
<?php
$host = gethostbyname('github.com');
$loop = new UVLoop();
$ssl = new UVSSL($loop);
$ssl->html = '';
$ssl->connect($host, 443, function($ssl){
    $ssl->setSSLHandshakeCallback(function($ssl){
        echo "handshake: ok\n";
        $ssl->setCallback(function($ssl, $recv){
            $ssl->html.=$recv;
        }, function(){}, function($ssl){            
            if(($pos = strpos($ssl->html, "\r\n\r\n")) !== false){
               $header = substr($ssl->html, 0, $pos);
               preg_match('/Status:\s(\d+)/i', $header, $match);
               echo "http status: {$match[1]}\n";
            }
            $ssl->close();
        });
        $request = "GET /RickySu/php_ext_uv HTTP/1.0\r\nUser-Agent: UVSSL\r\nAccept: */*\r\nHost: github.com\r\n\r\n";
        $ssl->write($request);
    });
});

$loop->run();
?>
--EXPECT--
handshake: ok
http status: 200
