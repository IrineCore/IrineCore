USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_DailyVisit]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_DailyVisit](
	[BasisDate] [date] NOT NULL,
	[D15NoVisitCount] [int] NOT NULL,
	[D30NoVisitCount] [int] NOT NULL,
	[D31NoVisitCount] [int] NOT NULL,
	[HardUserCnt] [int] NOT NULL,
	[HardUserPlayTimeMinAvg] [int] NOT NULL,
	[LightUserCnt] [int] NOT NULL,
	[LightUserPlayTimeMinAvg] [int] NOT NULL,
 CONSTRAINT [PK_ST_DailyVisit] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
