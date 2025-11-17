pipeline {
    // No default agent, each stage specifies its own
    agent none

    stages {

        // Stage 1: Build on the Raspberry Pi agent
        stage('Build') {
            agent { label 'host' }
            steps {
                script {
                    // Run commands inside a Python Docker container
                    checkout scm
                    sh 'uname -a'
                    echo "This step runs on the Jenkins controller"
                }
            }
        }

        // Stage 2: Run a step on the Jenkins controller
        stage('Test') {
            agent { label 'rpi3b' }
            steps {
                script {
                    docker.image('python:3.10.19-trixie').inside {
                        echo "Running script: ${params.TO_RUN}"
                        sh 'python --version'
                    }
                    
                }
            }
        }
    }
}
