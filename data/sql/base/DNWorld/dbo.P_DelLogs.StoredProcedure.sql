USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelLogs]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 35
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-14
description : ??? ?????. (?? & ?? ??? ??? ???? ?? ? ??? ?????.)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
16 = MaterializedItems ????? ???? ?? ?? at 2010-06-30
17 = MaterializedItems ????? ???? ??? ?? at 2010-07-02
19 = TeacherAndPupilLogs ???? ??? ????? ?? at 2010-08-03
20 = CharacterLoginLogs, ???? ???? ??? ????? ?? at 2010-12-28
21 = PointLogs ???? ??? ????? ?? at 2011-03-30
22 = FieldLogs ???? ??? ????? ??     by 20110419.wgkim
23 = FieldDeleteLogs, FieldAttachmentDeleteLogs ???? ?? ??? ????? ??     by 20111007.wgkim
24 = PetSkillLogs ???? ?? ??? ????? ??     by ??? at 2012-01-02
25~26 = SkillPointLogs ??? ??? ?? ??     by ??? at 2012-01-03
27~28 = PVPEndLogs ???? ?? ?? by ??? at 2012-03-08
27~30 = ???? ?? ??? ??? ?? ?? by ??? at 2012-07-27
**/

CREATE PROCEDURE [dbo].[P_DelLogs]
	@dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @i int
	, @j int
	, @nvcStmt nvarchar(4000)
	, @nvcLogTableName nvarchar(128)
	, @nvcDTColumnName nvarchar(128)
	, @nchLogTableSuffix nchar(1)
	, @intAffectedRows int = 1
	, @dt2BasisDate datetime2(0) = DATEADD(DAY, DATEDIFF(DAY, 0, @dtmBasisDate), 0)
	, @inbStageStartID bigint
	, @inbPVPStartID bigint
	, @inbNestGateStartID bigint;

DECLARE @tblTables table (Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, LogTableName sysname NOT NULL, DTColumnName sysname NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblTables (LogTableName, DTColumnName)
	VALUES (N'CharacterLevelLogs', N'LogDate')
		, (N'CharacterLogoutLogs', N'LogoutDate')
		, (N'CoinLogs', N'LogDate')
		, (N'FatigueLogs', N'LogDate')
		, (N'SkillLogs', N'LogDate')
		, (N'MapStayLogs', N'ExitDate')
		, (N'ItemLogs', N'LogDate')
		, (N'CashRebirthCoinLogs', N'LogDate')
		, (N'TeacherAndPupilLogs', N'LogDate')
		, (N'CharacterLoginLogs', N'LoginDate')
		, (N'PointLogs', N'LogDate')
		, (N'FieldDeleteLogs', N'LogDate')
		, (N'FieldAttachmentDeleteLogs', N'LogDate')
		, (N'PetSkillLogs', N'BasisDate')
		, (N'NestDeathLogs', N'LogDate')
		, (N'PCRoomCharacterLogoutLogs', N'LogoutDate')
		, (N'PCRoomCharacterLoginLogs', N'LoginDate');

	SELECT @i = 1, @j = @@ROWCOUNT, @nchLogTableSuffix = CAST(dbo.FN_LogTableSuffix(DATEADD(DAY, -1, @dt2BasisDate)) AS nchar(1));

	WHILE @i <= @j
	BEGIN
		SELECT @nvcLogTableName = LogTableName + @nchLogTableSuffix, @nvcDTColumnName = DTColumnName
		FROM @tblTables
		WHERE Seq = @i;

		SET @nvcStmt = N'
IF NOT EXISTS (SELECT *  FROM dbo.' + @nvcLogTableName + N' WITH (NOLOCK) WHERE ' + @nvcDTColumnName + N' >= @dt2BasisDate)
	TRUNCATE TABLE dbo.' + @nvcLogTableName + N';';

		EXEC sp_executesql @nvcStmt, N'@dt2BasisDate datetime2(0)', @dt2BasisDate = @dt2BasisDate;

		SET @i += 1;
	END

	SELECT TOP (1) @inbStageStartID = StageStartID
	FROM dbo.StageStartIDs WITH (NOLOCK)
	WHERE StageStartDate >= @dt2BasisDate
	ORDER BY StageStartID;

	SELECT TOP (1) @inbPVPStartID = PVPStartID
	FROM dbo.PVPStartIDs WITH (NOLOCK)
	WHERE PVPStartDate >= @dt2BasisDate
	ORDER BY PVPStartID;

	SELECT TOP (1) @inbNestGateStartID = NestGateStartID
	FROM dbo.NestGateStartIDs WITH (NOLOCK)
	WHERE NestGateStartDate >= @dt2BasisDate
	ORDER BY NestGateStartID;

	SET @nvcStmt = N'
IF NOT EXISTS (SELECT *  FROM dbo.StageEndLogs' + @nchLogTableSuffix + N' WITH (NOLOCK) WHERE StageStartID >= @inbStageStartID)
	TRUNCATE TABLE dbo.StageEndLogs' + @nchLogTableSuffix + N';';
	EXEC sp_executesql @nvcStmt, N'@inbStageStartID bigint', @inbStageStartID = @inbStageStartID;

	SET @nvcStmt = N'
IF NOT EXISTS (SELECT *  FROM dbo.StageRewardLogs' + @nchLogTableSuffix + N' WITH (NOLOCK) WHERE StageStartID >= @inbStageStartID)
	TRUNCATE TABLE dbo.StageRewardLogs' + @nchLogTableSuffix + N';';
	EXEC sp_executesql @nvcStmt, N'@inbStageStartID bigint', @inbStageStartID = @inbStageStartID;

	SET @nvcStmt = N'
IF NOT EXISTS (SELECT *  FROM dbo.PVPStartLogs' + @nchLogTableSuffix + N' WITH (NOLOCK) WHERE PVPStartID >= @inbPVPStartID)
	TRUNCATE TABLE dbo.PVPStartLogs' + @nchLogTableSuffix + N';';
	EXEC sp_executesql @nvcStmt, N'@inbPVPStartID bigint', @inbPVPStartID = @inbPVPStartID;
	
	SET @nvcStmt = N'
IF NOT EXISTS (SELECT *  FROM dbo.PVPEndLogs' + @nchLogTableSuffix + N' WITH (NOLOCK) WHERE PVPStartID >= @inbPVPStartID)
	TRUNCATE TABLE dbo.PVPEndLogs' + @nchLogTableSuffix + N';';
	EXEC sp_executesql @nvcStmt, N'@inbPVPStartID bigint', @inbPVPStartID = @inbPVPStartID;

	SET @nvcStmt = N'
IF NOT EXISTS (SELECT *  FROM dbo.NestGateClearLogs' + @nchLogTableSuffix + N' WITH (NOLOCK) WHERE NestGateStartID >= @inbNestGateStartID)
	TRUNCATE TABLE dbo.NestGateClearLogs' + @nchLogTableSuffix + N';';
	EXEC sp_executesql @nvcStmt, N'@inbNestGateStartID bigint', @inbNestGateStartID = @inbNestGateStartID;

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
