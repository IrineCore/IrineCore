USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCUCount]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetCUCount]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetCUCount] AS' 
END
GO
/**
version : 18
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-01
description : ??? ?? ??? ?? ?????.
result set : SnapshotDate/datetime2(0), WorldID/int, CUCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
12 = VIX_CUTotals ?? ???? ??? ?? at 2010-08-04
14 = WorldID ?? 0? ??? ??? ?????? ?? at 2010-11-23
15 = OPTION ??     by ??? at 2012-03-16
**/

ALTER PROCEDURE [dbo].[P_GetCUCount]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;

DECLARE @dt2SnapshotDate datetime2(0) = GETDATE();

IF OBJECT_ID(N'CUCount', N'U') IS NULL
BEGIN
	EXEC ('CREATE TABLE dbo.CUCount (SnapshotDate datetime2(0) NOT NULL, WorldID int NOT NULL, CUCount int NOT NULL, ChannelPartnerCode tinyint       CONSTRAINT DF_CUCount_ChannelPartnerCode DEFAULT (0)  NOT NULL)');
	EXEC ('ALTER TABLE dbo.CUCount ADD CONSTRAINT PK_CUCount PRIMARY KEY CLUSTERED (SnapshotDate, WorldID, ChannelPartnerCode) ON DataFG1');
END

INSERT dbo.CUCount (SnapshotDate, WorldID, CUCount, ChannelPartnerCode)
SELECT @dt2SnapshotDate, WorldID, COUNT(*), ISNULL(ChannelPartnerCode,0)
FROM dbo.DNAuth WITH (NOLOCK)
WHERE CertifyingStep = 2
GROUP BY WorldID, ChannelPartnerCode;

RETURN 0;
GO
