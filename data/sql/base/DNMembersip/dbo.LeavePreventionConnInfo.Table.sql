USE [dnmembership]
GO
/****** Object:  Table [dbo].[LeavePreventionConnInfo]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[LeavePreventionConnInfo]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[LeavePreventionConnInfo](
	[AccountID] [int] NOT NULL,
	[PCRoomLoginDateCount] [tinyint] NOT NULL,
	[HomeLoginDateCount] [tinyint] NOT NULL,
	[LoginDateCount] [tinyint] NOT NULL,
	[AveragePlayTime] [smallint] NOT NULL,
	[TotalSessionCount] [smallint] NOT NULL,
 CONSTRAINT [PK_LeavePreventionConnInfo] PRIMARY KEY CLUSTERED 
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
