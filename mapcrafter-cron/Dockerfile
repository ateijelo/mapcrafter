FROM mapcrafter-fix:118

ADD generate_map.sh /
ENTRYPOINT ["/usr/sbin/crond", "-f", "-l", "8"]
