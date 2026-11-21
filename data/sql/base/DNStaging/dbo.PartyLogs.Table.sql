USE [dnstaging]
GO
/****** Object:  Table [dbo].[PartyLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PartyLogs](
	[PartyStartDate] [datetime2](0) NOT NULL,
	[PartyEndDate] [datetime2](0) NULL,
	[PartyStartID] [bigint] NOT NULL,
	[WorldID] [int] NULL
) ON [PRIMARY]
GO
