ARG IDF_TAG
from espressif/idf:${IDF_TAG} as Base

ENV HOMEKIT_PATH="/opt/esp/homekit-sdk"
RUN git clone --recursive https://github.com/espressif/esp-homekit-sdk.git ${HOMEKIT_PATH}
RUN /opt/esp/entrypoint.sh python -m pip install -r ${IDF_PATH}/tools/esp_prov/requirements.txt
# RUN "sh -C 'source /opt/esp/idf/export.sh; python -m pip install -r ${IDF_PATH}/tools/esp_prov/requirements.txt'"

ENTRYPOINT ["/opt/esp/entrypoint.sh"]
CMD ["/bin/bash"]
