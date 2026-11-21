USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_DailyDashboard]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_DailyDashboard](
	[BasisDate] [date] NOT NULL,
	[RUCount] [int] NOT NULL,
	[UVCount] [int] NOT NULL,
	[RevisitType1] [int] NOT NULL,
	[RevisitType2] [int] NOT NULL,
	[AvgDT] [smallint] NOT NULL,
	[AvgDTForNewbie] [smallint] NOT NULL,
	[SaleAmount] [bigint] NOT NULL,
	[NationalityCode] [tinyint] NULL,
	[LeaveAccountCount] [int] NOT NULL,
	[RevisitCount3] [int] NOT NULL,
	[UVPCRoom] [int] NULL,
	[DTForPCRoom] [bigint] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ST_DailyDashboard_BasisDate]    Script Date: 11/14/2026 9:00:07 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ST_DailyDashboard_BasisDate] ON [dbo].[ST_DailyDashboard]
(
	[BasisDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
