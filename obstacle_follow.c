void obstacle_follow_task(void)
{
    unsigned int d;

    d = Obstacle_GetDistanceMM();

    printf("Distance = %u mm\r\n", d);

    if (!Obstacle_IsReady())
    {
        motor_stop();
        return;
    }

    if (d > 0 && d < 70)
    {
        printf("Backward\r\n");
        motor_backward(10);
    }
    else if (d >= 70 && d < 120)
    {
        printf("Stop\r\n");
        motor_stop();
    }
    else if (d >= 120 && d < 220)
    {
        printf("Forward\r\n");
        motor_forward(10);
    }
    else
    {
        printf("Idle stop\r\n");
        motor_stop();
    }
}
while (1)
{
    if (current_mode == Auto_mode)
    {
        auto_mode_task(path_number, &intersection_count);
    }
    else if (current_mode == Manual_mode)
    {
        manual_command_execute(0);
    }
    else if (current_mode == Obstacle_mode)
    {
        obstacle_follow_task();
    }

    delayms(50);
}
