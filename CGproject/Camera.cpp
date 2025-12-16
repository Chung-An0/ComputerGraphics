#include "Camera.h"

Camera::Camera() {
    mode = CameraMode::FIRST_PERSON;

    // 1인칭 초기 위치 (파울라인 뒤)
    position = vec3(0.0f, 1.6f, 2.0f);  // 사람 눈높이
    pitch = -5.0f;   // 살짝 아래를 봄
    yaw = -90.0f;    // 앞을 봄 (-Z 방향)

    // 공 추적 카메라 초기값
    followDistance = 2.5f;
    followHeight = 1.0f;

    // 플레이어 좌우 위치 (레인 중앙)
    playerX = 0.0f;
}

void Camera::Apply() {
    if (mode == CameraMode::FIRST_PERSON) {
        UpdateFirstPerson();
    }

    vec3 target = position + GetForward();
    gluLookAt(
        position.x, position.y, position.z,
        target.x, target.y, target.z,
        0.0f, 1.0f, 0.0f
    );
}

void Camera::UpdateFirstPerson() {
    // 플레이어 X 위치 반영
    position.x = playerX;
}

void Camera::UpdateBallFollow(vec3 ballPos, vec3 ballDir) {
    if (mode != CameraMode::BALL_FOLLOW) return;

    // 공 뒤쪽 + 위에서 따라감
    vec3 behindOffset = -normalize(ballDir) * followDistance;
    vec3 heightOffset = vec3(0.0f, followHeight, 0.0f);

    position = ballPos + behindOffset + heightOffset;
    targetPosition = ballPos;
    targetDirection = ballDir;

    // 카메라가 공보다 앞을 봐야 핀이 보임
    vec3 lookTarget = ballPos + normalize(ballDir) * 5.0f;

    // pitch와 yaw 계산 (공 앞쪽을 바라보도록)
    vec3 dir = normalize(lookTarget - position);
    pitch = degrees(asin(dir.y));
    yaw = degrees(atan2(dir.x, -dir.z)) - 90.0f;
}

// 공 위치를 기준으로 위에서 내려다보는 뷰를 갱신한다.
// 위쪽 높이와 뒤쪽 거리 값을 조정하여 원하는 프레이밍을 얻을 수 있다.
void Camera::UpdateTopView(vec3 ballPos) {
    // 일정한 높이에서 공을 따라가며 위에서 내려다본다.
    // 카메라 위치는 공보다 일정 z 오프셋과 y 높이를 유지한다.
    float topHeight = 4.0f;
    float topOffsetZ = 1.0f;
    position = vec3(ballPos.x, topHeight, ballPos.z + topOffsetZ);

    // 공을 바라보는 방향 계산
    vec3 lookTarget = ballPos;
    vec3 dir = normalize(lookTarget - position);
    // pitch와 yaw 갱신: dir의 각도를 변환
    pitch = degrees(asin(dir.y));
    yaw = degrees(atan2(dir.x, -dir.z)) - 90.0f;
}

// 공 위치를 기준으로 레인 측면에서 바라보는 뷰를 갱신한다.
void Camera::UpdateSideView(vec3 ballPos) {
    // 레인 폭 기준으로 측면 x 위치 결정
    float sideOffsetX = (LANE_WIDTH / 2.0f) + GUTTER_WIDTH + 0.3f;
    // 약간 위에서 공과 함께 이동
    float sideHeight = 1.5f;
    position = vec3(sideOffsetX, sideHeight, ballPos.z);

    // 공을 바라보는 방향 계산
    vec3 lookTarget = ballPos;
    vec3 dir = normalize(lookTarget - position);
    pitch = degrees(asin(dir.y));
    yaw = degrees(atan2(dir.x, -dir.z)) - 90.0f;
}

void Camera::LookUp(float amount) {
    pitch += amount;
    // 제한: 너무 위나 아래로 안 보게
    if (pitch > 60.0f) pitch = 60.0f;
}

void Camera::LookDown(float amount) {
    pitch -= amount;
    if (pitch < -60.0f) pitch = -60.0f;
}

void Camera::MoveLeft(float amount) {
    playerX -= amount;
    // 레인 범위 제한
    float limit = LANE_WIDTH / 2.0f - BALL_RADIUS;
    if (playerX < -limit) playerX = -limit;
}

void Camera::MoveRight(float amount) {
    playerX += amount;
    float limit = LANE_WIDTH / 2.0f - BALL_RADIUS;
    if (playerX > limit) playerX = limit;
}

void Camera::SetMode(CameraMode newMode) {
    mode = newMode;

    if (mode == CameraMode::FIRST_PERSON) {
        // 1인칭으로 돌아갈 때 초기화
        position = vec3(playerX, 1.6f, 2.0f);
        pitch = -5.0f;
        yaw = -90.0f;
    }
    else if (mode == CameraMode::TOP_VIEW) {
        // 초기화는 최소한의 값만 설정한다. 실제 위치는 UpdateTopView에서 설정.
        // pitch/yaw는 위에서 아래로 내려다보도록 기본값을 준다.
        pitch = -90.0f;
        yaw = -90.0f;
    }
    else if (mode == CameraMode::SIDE_VIEW) {
        // 측면에서 바라보도록 초기 yaw/pitch 설정 (정면은 -90, 측면은 180)
        pitch = 0.0f;
        yaw = 180.0f;
    }
}

vec3 Camera::GetForward() {
    vec3 forward;
    forward.x = cos(radians(yaw)) * cos(radians(pitch));
    forward.y = sin(radians(pitch));
    forward.z = sin(radians(yaw)) * cos(radians(pitch));
    return normalize(forward);
}

vec3 Camera::GetRight() {
    return normalize(cross(GetForward(), vec3(0.0f, 1.0f, 0.0f)));
}

vec3 Camera::GetUp() {
    return normalize(cross(GetRight(), GetForward()));
}