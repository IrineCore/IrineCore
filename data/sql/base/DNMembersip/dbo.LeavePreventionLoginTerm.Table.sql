USE [dnmembership]
GO
/****** Object:  Table [dbo].[LeavePreventionLoginTerm]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[LeavePreventionLoginTerm]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[LeavePreventionLoginTerm](
	[AccountID] [int] NOT NULL,
	[LastLoginDate] [datetime] NOT NULL,
	[Term1] [smallint] NOT NULL,
	[Term2] [smallint] NOT NULL,
	[Term3] [smallint] NOT NULL,
	[Term4] [smallint] NOT NULL,
	[Term5] [smallint] NOT NULL,
	[Term6] [smallint] NOT NULL,
	[Term7] [smallint] NOT NULL,
	[Term8] [smallint] NOT NULL,
	[Term9] [smallint] NOT NULL,
	[Term10] [smallint] NOT NULL,
 CONSTRAINT [PK_LeavePreventionLoginTerm] PRIMARY KEY CLUSTERED 
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
