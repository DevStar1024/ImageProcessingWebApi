FROM python:3.9-slim

ENV PYTHONDONTWRITEBYTECODE 1
ENV PYTHONUNBUFFERED 1

WORKDIR /app

COPY ./mainApi/requirements.txt /requirements.txt

# additional dependencies
# netcat used for checking for resources on the network before start, see start.sh for use
RUN apt-get update && \
    apt-get install -y netcat-traditional

# Install dependencies to solve dependency bug: openslide
RUN apt install -y openslide-tools && \
    apt install -y build-essential && \
    apt install -y git && \
    apt install -y default-jdk && \
    pip install numpy==1.23.5 && \
    pip install -r /requirements.txt

RUN apt-get update && \
    apt-get install -y libopencv-dev


EXPOSE 8000
