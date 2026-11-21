USE [dnstaging]
GO
/****** Object:  Table [dbo].[ProcedureExecStats]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ProcedureExecStats](
	[BasisDate] [date] NOT NULL,
	[ServerName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[DatabaseName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[ProcedureName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CachedTime] [datetime2](0) NULL,
	[ExecutionCount] [bigint] NULL,
	[AvgElapsedTime] [bigint] NULL,
	[AvgWorkerTime] [bigint] NULL,
	[AvgLogicalReads] [bigint] NULL,
	[AvgLogicalWrites] [bigint] NULL,
	[TotalElapsedTime] [bigint] NULL,
	[MinElapsedTime] [bigint] NULL,
	[MaxElapsedTime] [bigint] NULL,
	[TotalWorkerTime] [bigint] NULL,
	[MinWorkerTime] [bigint] NULL,
	[MaxWorkerTime] [bigint] NULL,
	[TotalLogicalReads] [bigint] NULL,
	[MinLogicalReads] [bigint] NULL,
	[MaxLogicalReads] [bigint] NULL,
	[TotalLogicalWrites] [bigint] NULL,
	[MinLogicalWrites] [bigint] NULL,
	[MaxLogicalWrites] [bigint] NULL,
	[QueryPlan] [xml] NULL,
 CONSTRAINT [PK_ProcedureExecStats] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[ServerName] ASC,
	[DatabaseName] ASC,
	[ProcedureName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, FILLFACTOR = 100) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
