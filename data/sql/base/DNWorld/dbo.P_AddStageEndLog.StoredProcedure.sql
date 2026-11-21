USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddStageEndLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-06
description : ???? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = StageEndDate ?? NULL? ???? StageStartIDs ???? UPDATE??? ?? at 2011-01-11
**/

CREATE PROCEDURE [dbo].[P_AddStageEndLog]
	  @inbRoomID bigint --// ??? ? ID
	, @inyWhereToGoCode tinyint --// ???? ?? ? ?? ??. 1=???? ??, 2=??. ??? ?? ???? NULL ?????.
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbStageStartID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [StageStartIDs] ???? UPDATE???.*/
	SELECT TOP (1) @inbStageStartID = StageStartID
	FROM dbo.StageStartIDs WITH (NOLOCK)
	WHERE RoomID = @inbRoomID
	ORDER BY StageStartID DESC;

	UPDATE dbo.StageStartIDs
	SET	StageEndDate = GETDATE(), WhereToGoCode = @inyWhereToGoCode
	WHERE StageStartID = @inbStageStartID AND StageEndDate IS NULL;
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
