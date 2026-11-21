USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SendGuildWarRewardPoint]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-05-06

description :
	??? ?? ??? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	2 = ?? ??
	3 = PK ?? ?? ??     by 20110908.wgkim
	4 = PK ?? ?? ??     by 20110908.wgkim
	5 = input parameter (@insGuildWarScheduleID) ??, ?? ??? ?? ?? ??     by 20110919.wgkim
**/

CREATE PROCEDURE [dbo].[P_SendGuildWarRewardPoint]
	  @intGuildID int --// ?? ID
	, @insGuildWarScheduleID smallint --// ??? ??
	, @inbPoint bigint --// ??? ??? ?
	, @intMapID int --// ? ID
	, @vchIP varchar(15) --// IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int, @dt2Now datetime2(0) = GETDATE(), @dtBasisDate smalldatetime;

DECLARE @Temp_GuildMembers TABLE (CharacterID bigint, Point int null);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	IF @inbPoint <= 0 RETURN 0;

	SELECT @dtBasisDate = RewardEndDate
	  FROM dbo.GuildWarSchedule WITH (NOLOCK)
	 WHERE GuildWarScheduleID = @insGuildWarScheduleID;

	-- ?? ??
	INSERT @Temp_GuildMembers (CharacterID, Point)
	SELECT GM.CharacterID, P.Point
	  FROM dbo.GuildMembers GM WITH (NOLOCK)
	       LEFT OUTER JOIN dbo.Points P WITH (NOLOCK) ON (GM.CharacterID = P.CharacterID AND P.PointCode = 5)
	 WHERE GM.GuildID = @intGuildID
	   AND GM.RegisterDate < @dtBasisDate;;

	-- ??? ???? ?? (UPSERT)
	UPDATE dbo.Points
	   SET Point = P.Point + @inbPoint
	  FROM dbo.Points P
	       INNER JOIN @Temp_GuildMembers GM ON (P.CharacterID = GM.CharacterID)
	 WHERE P.PointCode = 5
	   AND GM.Point IS NOT NULL;

	INSERT dbo.Points (CharacterID, PointCode, Point)
	SELECT GM.CharacterID, 5, @inbPoint
	  FROM @Temp_GuildMembers GM
	 WHERE GM.Point IS NULL;

	-- ??? ?? ??? ??
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

	IF @intLogTableSuffix = 0
		INSERT dbo.PointLogs0 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		SELECT @dt2Now, CharacterID, 5, ISNULL(Point, 0), ISNULL(Point, 0) + @inbPoint, @intMapID, dbo.FN_IP2IPNumber(@vchIP) FROM @Temp_GuildMembers;

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.PointLogs1 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		SELECT @dt2Now, CharacterID, 5, ISNULL(Point, 0), ISNULL(Point, 0) + @inbPoint, @intMapID, dbo.FN_IP2IPNumber(@vchIP) FROM @Temp_GuildMembers;

	ELSE
		INSERT dbo.PointLogs2 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		SELECT @dt2Now, CharacterID, 5, ISNULL(Point, 0), ISNULL(Point, 0) + @inbPoint, @intMapID, dbo.FN_IP2IPNumber(@vchIP) FROM @Temp_GuildMembers;

END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
