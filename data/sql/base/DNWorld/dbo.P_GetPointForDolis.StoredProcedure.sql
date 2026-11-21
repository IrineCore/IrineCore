USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPointForDolis]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-04-10
description : ???? ?? ??? ?? ?????.(DOLIS)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetPointForDolis]
	  @inbCharacterID bigint --// ??? ID
	, @inbPVPLadderPoint bigint OUTPUT --// ?? ??? ?
	, @inbGuildWarPoint bigint OUTPUT --// ?? ?????
	, @inbGuildWarOpeningPoint bigint OUTPUT --// ??? ?? ???
	, @inbDealerPoint bigint OUTPUT --// ?? ?? ???
	, @inbKingdomPoint bigint OUTPUT --// ?? ?? ???
	, @inbFreedomPoint bigint OUTPUT --// ?? ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ???? ?? ???? ?????.*/
	SELECT @inbDealerPoint =  ISNULL(SUM(CASE WHEN PointCode = 2 THEN Point END), 0)
		 , @inbKingdomPoint =  ISNULL(SUM(CASE WHEN PointCode = 3 THEN Point END), 0)
		 , @inbFreedomPoint =  ISNULL(SUM(CASE WHEN PointCode = 4 THEN Point END), 0)
		 , @inbGuildWarPoint = ISNULL(SUM(CASE WHEN PointCode = 5 THEN Point END), 0)
	FROM dbo.Points WITH(NOLOCK, INDEX(PK_Points), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	GROUP BY CharacterID
	OPTION(KEEPFIXED PLAN);

	/**_# ?? ?? ???? ?????.*/
	SELECT @inbGuildWarOpeningPoint = ISNULL(OpeningPoint, 0)
	FROM dbo.VIX_GuildWarOpeningPointForCharacter WITH(NOEXPAND, NOLOCK)
	WHERE CharacterID = @inbCharacterID
	OPTION(KEEPFIXED PLAN);

	/**_# PVP ?? ???? ?????.*/
	SELECT @inbPVPLadderPoint = ISNULL(PVPLadderPoint, 0)
	FROM dbo.CharacterStatus WITH(NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION(KEEPFIXED PLAN);

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
