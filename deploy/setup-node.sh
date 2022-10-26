#!/bin/sh
# this file is intended of setting up a kryptokrona node on a vps
# copy this file to your vps and make it executable

NGINX_PROJECT_DIR="/etc/nginx/sites-enabled"
CURRENT_DIR=$(pwd)
DOMAIN="example.com"
EMAIL="foo@bar.com"

echo ""
echo "###### UPDATING HEADERS ######"
echo ""
sudo apt-get update

echo ""
echo "###### INSTALLING DEPENDENCIES ######"
echo ""
sudo apt-get -y install \
    ca-certificates \
    curl \
    gnupg \
    lsb-release \
    git \
    curl \
    nginx \
    certbot \
    python3-certbot-nginx \
    p7zip-full

echo ""
echo "###### SETUP KEYRING FOR DOCKER ######"
echo ""
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg

echo ""
echo "###### UPDATING SOURCES LIST FOR DOCKER ######"
echo ""
echo \
"deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu \
$(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

echo ""
echo "###### INSTALLING DOCKER ######"
echo ""
sudo apt-get update
sudo apt-get install -y docker-ce docker-ce-cli containerd.io

echo ""
echo "###### CLONE KRYPTOKRONA REPOSITORY ######"
echo ""
if [ -f "kryptokrona" ]; then
    echo "kryptokrona repository exists. Skipping..."
    (cd kryptokrona && git pull)
else
    git clone https://github.com/kryptokrona/kryptokrona.git
fi

echo ""
echo "###### DOWNLOADING EXISTING BLOCKS FROM BOOTSTRAP ######"
echo ""
if [ -f "bootstrap.7z" ]; then
    echo "bootstrap.7z exists. No need to download. Skipping..."
elif [ -d "bootstrap" ]; then
    echo "boostrap directory exists. No need to extract it. Skipping..."
else
    curl http://wasa.kryptokrona.se/xkr-bootstrap/bootstrap-20220426.7z --output bootstrap.7z

    echo ""
    echo "###### EXTRACING BOOSTRAP ######"
    echo ""
    7za x bootstrap.7z -o./bootstrap
fi

echo ""
echo "###### BULDING DOCKER IMAGE ######"
echo ""
(cd ./kryptokrona && docker build -f ./deploy/Dockerfile -t kryptokrona/kryptokrona-node .)

echo ""
echo "###### CREATING DOCKER NETWORK ######"
echo ""
docker network create kryptokrona

echo ""
echo "###### RUNNING DOCKER CONTAINER ######"
echo ""
docker run -d -p 11898:11898 --volume=$CURRENT_DIR/bootstrap/.kryptokrona:/usr/src/kryptokrona/build/src/blockloc --network=kryptokrona kryptokrona/kryptokrona-node 

echo ""
echo "###### SETTING UP NGINX AND LET'S ENCRYPT ######"
echo ""

# setup configuration file
sudo tee -a $DOMAIN > /dev/null <<EOT
server {
    server_name         $DOMAIN;

    location / {
        proxy_pass http://127.0.0.1:11898;
        proxy_set_header Host \$host;
        proxy_set_header X-Real-IP \$remote_addr;
        proxy_set_header X-Forwarded-For \$proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto \$scheme;
    }
}

server {
    if (\$host = $DOMAIN) {
        return 301 https://\$host\$request_uri;
    }

    listen              80;
    listen              [::]:80;
    server_name         $DOMAIN;
    return              404;
}
EOT

echo ""
echo "###### COPY NGINX CONFIG TO NGINX ######"
echo ""
sudo cp $CURRENT_DIR/$DOMAIN $NGINX_PROJECT_DIR/$DOMAIN

echo ""
echo "###### CONFIGURE CERTBOT ######"
echo ""
sudo certbot --nginx -d $DOMAIN --non-interactive --agree-tos -m $EMAIL

echo ""
echo "###### RELOAD AND RESTART NGINX ######"
echo ""
sudo systemctl reload nginx
sudo systemctl restart nginx
