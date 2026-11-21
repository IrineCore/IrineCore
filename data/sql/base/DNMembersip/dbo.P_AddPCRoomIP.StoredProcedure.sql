USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPCRoomIP]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddPCRoomIP]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddPCRoomIP] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-04
description : PC? IP? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101160 = ?? ??? IP??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_AddPCRoomIP]
	  @vchIPs varchar(max) --// ,? ?????? IP ?? ??? (?? 1,000?)
	, @vchPCRoomGrades varchar(max) = '' --// ,? ?????? PC? ?? ???
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
	/**_# [PCRoomIPs] ???? INSERT???.*/
	INSERT dbo.PCRoomIPs (IP, PCRoomGrade)
	SELECT dbo.FN_IP2IPNumber(A.string), CAST(ISNULL(B.string, 1) AS tinyint)
	FROM dbo.TF_Split(@vchIPs, ',') A
		LEFT OUTER JOIN dbo.TF_Split(@vchPCRoomGrades, ',') B ON A.seq = B.seq
	WHERE A.string > '' AND B.string > '';
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_PCRoomIPs%'
		SET @intReturnValue = 101160;

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
