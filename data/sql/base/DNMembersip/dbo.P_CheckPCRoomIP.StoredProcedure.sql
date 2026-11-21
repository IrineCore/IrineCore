USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_CheckPCRoomIP]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_CheckPCRoomIP]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_CheckPCRoomIP] AS' 
END
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-04
description : PC? IP?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history : 
5 = PC? IP? ?? ??? @inyPCRoomGrade ?? 0 ?? return ??? ??     by 20110421.wgkim
**/

ALTER PROCEDURE [dbo].[P_CheckPCRoomIP]
	  @vchIP varchar(15)--// IP??
	, @bitPCRoomIPFlag bit = NULL OUTPUT --// ??? IP??? PC? IP? ???? ??? ??. 0=PC? IP ??, 1=PC? IP
	, @inyPCRoomGrade tinyint = 1 OUTPUT --// PC?? ?? PC?? ?? (SNDA) 1=??, 2=?? ??? ????, 3=?? ??? ????, 4=?? ??? ????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @nvcStmt nvarchar(max);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @inyPCRoomGrade = PCRoomGrade
	FROM dbo.PCRoomIPs
	WHERE IP = dbo.FN_IP2IPNumber(@vchIP);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @bitPCRoomIPFlag = 0;
		SET @inyPCRoomGrade = 0;
	END
	ELSE
		SET @bitPCRoomIPFlag = 1;

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
