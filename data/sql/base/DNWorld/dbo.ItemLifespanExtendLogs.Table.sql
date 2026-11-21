USE [dnworld]
GO
/****** Object:  Table [dbo].[ItemLifespanExtendLogs]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemLifespanExtendLogs](
	[ItemLifespanExtendLogID] [int] IDENTITY(1,1) NOT NULL,
	[ExtendedLifespan] [int] NOT NULL,
	[LogDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_ItemLifespanExtendLogs] PRIMARY KEY CLUSTERED 
(
	[ItemLifespanExtendLogID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
