USE [dnmembership]
GO
/****** Object:  Table [dbo].[AccountConnectNoticeQueue]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[AccountConnectNoticeQueue]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[AccountConnectNoticeQueue](
	[AccountID] [int] NOT NULL,
	[RegistedDate] [datetime2](0) NOT NULL
) ON [DataFG1]
END
GO
