USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPoint]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-03-04
description : ??? ??? ?????.
result set : PointCode/tinyint, Point/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
4 = @vchPointCode = '' ??, ??? ????? ?? by ??? at 2013-01-31
**/

CREATE PROCEDURE [dbo].[P_GetListPoint]
	  @inbCharacterID bigint --// ??? ID
	, @vchPointCode varchar(100) = '' --// ,? ?????? PointCode ???
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
	IF @vchPointCode = ''
		SELECT P.PointCode, P.Point
		FROM dbo.Points P WITH (NOLOCK, INDEX(PK_Points), FORCESEEK)
		WHERE P.CharacterID = @inbCharacterID
		OPTION (KEEPFIXED PLAN);

	ELSE
		SELECT P.PointCode, P.Point
		FROM dbo.TF_Split(@vchPointCode, ',') F
			INNER JOIN dbo.Points P WITH (NOLOCK, INDEX(PK_Points), FORCESEEK) ON CAST(F.string AS tinyint) = P.PointCode
		WHERE P.CharacterID = @inbCharacterID
		OPTION (KEEPFIXED PLAN);
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
