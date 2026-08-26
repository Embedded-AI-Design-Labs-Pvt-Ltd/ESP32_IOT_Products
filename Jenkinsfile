# ESP32-C3 IoT Platform — Jenkins CI/CD
# Canonical copy lives here and in ci/jenkins/Jenkinsfile.

pipeline {
  agent any
  options {
    timestamps()
    buildDiscarder(logRotator(numToKeepStr: '30'))
  }
  environment {
    IOT_VERSION = "1.0.0"
  }
  stages {
    stage('Host unit tests (no hardware)') {
      steps {
        sh '''
          cmake -S tests/host -B host_build
          cmake --build host_build
          ctest --test-dir host_build --output-on-failure
        '''
      }
    }
    stage('Format gate') {
      steps {
        sh '''
          if command -v clang-format >/dev/null 2>&1; then
            find components main tests/host -name '*.c' -o -name '*.h' | xargs clang-format --dry-run --Werror
          else
            echo "clang-format not installed — skip"
          fi
        '''
      }
    }
    stage('Static analysis') {
      steps {
        sh '''
          if command -v cppcheck >/dev/null 2>&1; then
            cppcheck --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem \
              -I components/iot_core/include components main
          else
            echo "cppcheck not installed — skip"
          fi
        '''
      }
    }
    stage('ESP-IDF firmware (Docker)') {
      steps {
        sh '''
          docker build -f docker/Dockerfile.idf -t esp32c3-iot-idf .
          docker run --rm -v "$PWD":/project -w /project esp32c3-iot-idf bash -lc "idf.py set-target esp32c3 && idf.py build"
        '''
      }
    }
    stage('Simulation image') {
      steps {
        sh 'docker build -f docker/Dockerfile.sim -t esp32c3-iot-sim .'
      }
    }
    stage('HTML docs') {
      steps {
        sh 'python tools/gui/generate_docs.py || python3 tools/gui/generate_docs.py'
        archiveArtifacts artifacts: 'docs/html/**', allowEmptyArchive: true
      }
    }
    stage('Package') {
      steps {
        sh '''
          mkdir -p dist
          if [ -f build/esp32c3_iot_platform.bin ]; then
            cp build/esp32c3_iot_platform.bin dist/esp32c3_iot_platform-${IOT_VERSION}.bin
          fi
          echo ${IOT_VERSION} > dist/VERSION
        '''
        archiveArtifacts artifacts: 'dist/**', allowEmptyArchive: true
      }
    }
  }
  post {
    failure {
      echo 'CI failed — do not publish firmware.'
    }
  }
}
